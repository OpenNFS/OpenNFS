// Interpolated values from the vertex shaders
in vec2 envUV;
in vec2 UV;

// Diffuse and Specular
in vec3 surfaceNormal;
in vec3 toLightVector[MAX_CAR_CONTRIB_LIGHTS];
in vec3 toCameraVector;

// CSM inputs
in vec3 fragPosWorldSpace;
in float clipSpaceZ;

flat in uint texIndex;
flat in uint polyFlag;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2DArray textureArray;
uniform sampler2D envMapTextureSampler;
uniform sampler2D carTextureSampler;
uniform sampler2DArrayShadow shadowMapArray;

uniform vec4 lightColour[MAX_CAR_CONTRIB_LIGHTS];
uniform vec3 attenuation[MAX_CAR_CONTRIB_LIGHTS];

uniform vec4 carColour;
uniform vec4 carSecondaryColour;
uniform float shineDamper;
uniform float reflectivity;
uniform float envReflectivity;

uniform bool multiTextured;
uniform bool polyFlagged;

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
    vec4 carTexColor = multiTextured ? texture(textureArray, vec3(UV, texIndex)).rgba : texture(carTextureSampler, UV ).rgba;
    if (carTexColor.a < 0.35) {
        color = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    } else if ((carTexColor.a < 0.8 && carTexColor.a > 0.75)) {
        carTexColor = carTexColor * carSecondaryColour;
        if (carColour.a > 0.0f) {
            carTexColor.r = carTexColor.r / carSecondaryColour.a;
            carTexColor.g = carTexColor.g / carSecondaryColour.a;
            carTexColor.b = carTexColor.b / carSecondaryColour.a;
        }
    } else if (carTexColor.a < 0.75){
        carTexColor = carTexColor * carColour;
        if (carColour.a > 0.0f) {
            carTexColor.r = carTexColor.r / carColour.a;
            carTexColor.g = carTexColor.g / carColour.a;
            carTexColor.b = carTexColor.b / carColour.a;
        }
    }
    vec4 envTexColor = texture( envMapTextureSampler, envUV ).rgba;

    vec3 unitNormal = normalize(surfaceNormal);
    vec3 unitVectorToCamera = normalize(toCameraVector);

    vec3 totalDiffuse = vec3(0.0f);
    vec3 totalSpecular = vec3(0.0f);

    for(int i = 0; i < MAX_CAR_CONTRIB_LIGHTS; ++i){
        float distance = length(toLightVector[i]);
        float attenFactor = attenuation[i].x + (attenuation[i].y * distance) + (attenuation[i].z * distance * distance);
        vec3 unitLightVector = normalize(toLightVector[i]);
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

    // CSM shadow calculation
    float shadow = ShadowCalculationWithBlending(fragPosWorldSpace, clipSpaceZ);

    // Apply shadow to diffuse (ambient remains lit)
    vec3 ambient = 0.3f * carTexColor.rgb;
    totalDiffuse = max(totalDiffuse * (1.0 - shadow), 0.4f);

	// Output color = color of the texture at the specified UV
    color = vec4(totalDiffuse, 1.0) * (carTexColor + envReflectivity * envTexColor) + vec4(totalSpecular * (1.0 - shadow), 1.0);
}
