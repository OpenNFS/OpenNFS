// Interpolated values from the vertex shaders
in vec2 UV;
flat in uint texIndex;
in vec4 nfsDataOut;
flat in uint debugDataOut;

// Diffuse and Specular
in vec3 surfaceNormal;

// CSM inputs
in vec3 fragPosWorldSpace;
in float clipSpaceZ;

// Ouput data
out vec4 color;

uniform bool useClassic;

uniform sampler2DArray textureArray;
uniform sampler2DArrayShadow shadowMapArray;
uniform float ambientFactor;
uniform vec3 cameraPosition;
uniform vec3 lightPosition[MAX_TRACK_CONTRIB_LIGHTS];
uniform vec4 lightColour[MAX_TRACK_CONTRIB_LIGHTS];
uniform vec3 attenuation[MAX_TRACK_CONTRIB_LIGHTS];
uniform vec3 spotlightPosition[MAX_SPOTLIGHTS];
uniform vec3 spotlightColour[MAX_SPOTLIGHTS];
uniform vec3 spotlightDirection[MAX_SPOTLIGHTS];
uniform float spotlightInnerCutOff[MAX_SPOTLIGHTS];
uniform float spotlightOuterCutOff[MAX_SPOTLIGHTS];
uniform float shineDamper;
uniform float reflectivity;

// Spotlight daylight washout constants
const float SPOTLIGHT_WASHOUT_RATE = 1.5;
const float SPOTLIGHT_MIN_VISIBILITY = 0.1;
const float SPOTLIGHT_MAX_VISIBILITY = 1.0;

// CSM uniforms
uniform mat4 lightSpaceMatrices[CSM_NUM_CASCADES];
uniform float cascadePlaneDistances[CSM_NUM_CASCADES];

// Select cascade based on view-space depth
int SelectCascade(float depth) {
    for (int i = 0; i < CSM_NUM_CASCADES; ++i) {
        if (depth < cascadePlaneDistances[i]) {
            return i;
        }
    }
    return CSM_NUM_CASCADES - 1;
}

// Calculate shadow with PCF for a specific cascade
float ShadowCalculationCSM(int cascadeIndex, vec3 worldPos) {
    // Transform to light space for selected cascade
    vec4 lightSpacePos = lightSpaceMatrices[cascadeIndex] * vec4(worldPos, 1.0);

    // Perspective divide
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Check if outside shadow map
    if (projCoords.z > 1.0) {
        return 0.0;
    }

    // Dynamic bias based on cascade (larger cascades need larger bias)
    float bias = 0.0005 * float(cascadeIndex + 1);
    float currentDepth = projCoords.z - bias;

    // 3x3 PCF with hardware shadow comparison
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            shadow += texture(shadowMapArray, vec4(projCoords.xy + offset, float(cascadeIndex), currentDepth));
        }
    }
    shadow /= 9.0;

    return 1.0 - shadow;  // Return shadow amount (0 = no shadow, 1 = full shadow)
}

// Calculate shadow with cascade blending for smooth transitions
float ShadowCalculationWithBlending(vec3 worldPos, float viewDepth) {
    int cascadeIndex = SelectCascade(viewDepth);
    float shadow = ShadowCalculationCSM(cascadeIndex, worldPos);

    // Blend between cascades at boundaries
    if (cascadeIndex < CSM_NUM_CASCADES - 1) {
        float blendStart = cascadePlaneDistances[cascadeIndex] - 5.0;  // Start blending 5 units before boundary

        if (viewDepth > blendStart) {
            float blendFactor = (viewDepth - blendStart) / 5.0;
            blendFactor = smoothstep(0.0, 1.0, blendFactor);

            float nextShadow = ShadowCalculationCSM(cascadeIndex + 1, worldPos);
            shadow = mix(shadow, nextShadow, blendFactor);
        }
    }

    return shadow;
}

void main(){
    // Get NFS texture colour
    vec4 texColour = texture(textureArray, vec3(UV, texIndex)).rgba;
    // Blend with NFS vertex colour
    vec4 nfsColor = texColour * nfsDataOut;

    // Discard if low alpha, no need to blend
    if (nfsColor.a < 0.1) {
        discard;
    }

    if (useClassic){
        color = nfsColor;
    } else {
        // Lay down summed diffuse and specular on top of NFS fragment colour
        vec3 unitNormal = normalize(surfaceNormal);
        vec3 unitVectorToCamera = normalize(cameraPosition - fragPosWorldSpace);

        vec3 totalDiffuse = vec3(0.0f);
        vec3 totalSpecular = vec3(0.0f);

        for (int i = 0; i < MAX_TRACK_CONTRIB_LIGHTS; ++i){
            // Calculate light vector in fragment shader (saves varying slots)
            vec3 toLightVector = lightPosition[i] - fragPosWorldSpace;
            float distance = length(toLightVector);
            float attenFactor = attenuation[i].x + (attenuation[i].y * distance) + (attenuation[i].z * distance * distance);
            vec3 unitLightVector = normalize(toLightVector);
            // Diffuse
            float nDot1 = dot(unitNormal, unitLightVector);
            float brightness = max(nDot1, 0.0);
            vec3 lightDirection = -unitLightVector;
            vec3 reflectedLightDirection = reflect(lightDirection, unitNormal);
            // Specular
            float specularFactor = dot(reflectedLightDirection, unitVectorToCamera);
            specularFactor = max(specularFactor, 0.0);
            float dampedFactor = pow(specularFactor, shineDamper);
            totalDiffuse += (brightness * lightColour[i].xyz)/attenFactor;
            totalSpecular += (dampedFactor * reflectivity * lightColour[i].xyz)/attenFactor;
        }
        totalDiffuse = max(totalDiffuse, ambientFactor);// Min brightness

        // Accumulate spotlight contributions
        vec3 spotlightContribution = vec3(0.0);
        for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
            // Skip inactive spotlights (zero colour indicates inactive)
            if (length(spotlightColour[i]) < 0.001)
                continue;

            // Calculate spotlight vector in fragment shader (saves varying slots)
            vec3 toSpotlight = spotlightPosition[i] - fragPosWorldSpace;

            // Check if lighting is inside the spotlight cone
            float theta = dot(normalize(toSpotlight), normalize(-spotlightDirection[i]));
            float epsilon = spotlightInnerCutOff[i] - spotlightOuterCutOff[i];
            float intensity = clamp((theta - spotlightOuterCutOff[i]) / epsilon, 0.0, 1.0);

            // Add spotlight contribution with distance attenuation
            float distance = length(toSpotlight);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
            spotlightContribution += intensity * attenuation * spotlightColour[i];
        }

        float shadow = ShadowCalculationWithBlending(fragPosWorldSpace, clipSpaceZ);
        vec3 ambient = 1.f * nfsColor.rgb;
        float spotlightVisibility = clamp(1.0 - ambientFactor * SPOTLIGHT_WASHOUT_RATE, SPOTLIGHT_MIN_VISIBILITY, SPOTLIGHT_MAX_VISIBILITY);
        vec3 lighting = (ambient + (1.0 - shadow) * (totalDiffuse + totalSpecular) + spotlightContribution * spotlightVisibility) * nfsColor.rgb;
        color = vec4(lighting, 1.0);
    }
}
