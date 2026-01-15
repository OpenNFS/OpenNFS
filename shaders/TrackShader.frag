// Interpolated values from the vertex shaders
in vec2 UV;
flat in uint texIndex;
in vec4 nfsDataOut;
flat in uint debugDataOut;

// Diffuse and Specular
in vec3 surfaceNormal;
in vec3 toLightVector[MAX_TRACK_CONTRIB_LIGHTS];
in vec3 toCameraVector;
// Spotlight
in vec3 toSpotlightVector;

// Fog
in vec3 worldPosition;
in vec4 viewSpace;
in vec4 lightSpace;

// Ouput data
out vec4 color;

uniform bool useClassic;

uniform sampler2DArray textureArray;
uniform sampler2D shadowMap;
uniform float ambientFactor;
uniform vec4 lightColour[MAX_TRACK_CONTRIB_LIGHTS];
uniform vec3 attenuation[MAX_TRACK_CONTRIB_LIGHTS];
uniform vec3 spotlightColour;
uniform vec3 spotlightDirection;
uniform float spotlightCutOff;
uniform float shineDamper;
uniform float reflectivity;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = lightSpace.xyz / lightSpace.w;
    // Depth map is in the range [0,1] and we also want to use projCoords to sample from the depth map so we transform the NDC coordinates to the range [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // The closest depth from the light's point of view
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // Projected vector's z coordinate equals the depth of the fragment from the light's perspective
    float currentDepth = projCoords.z;
    // Apply bias to remove acne, such that fragments are not incorrectly considered below the surface.
    float bias = 0.005;

    // Apply percentage closer filtering, to soften shadow edges (average neighbours by jittering projection coords)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            // Check whether currentDepth is higher than closestDepth and if so, the fragment is in shadow
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // Force the shadow value to 0.0 whenever the projected vector's z coordinate is larger than 1.0
    if (projCoords.z > 1.0) shadow = 0.0;

    return shadow;
}

void main(){
    // Get NFS texture colour
    vec4 texColour = texture(textureArray, vec3(UV, texIndex)).rgba;
    // Blend with NFS vertex colour
    vec4 nfsColor = texColour * nfsDataOut;

    // Discard if low alpha, no need to blend
    if (nfsColor.a < 0.1)
    discard;

    if (useClassic){
        color = nfsColor;
    } else {
        // Lay down summed diffuse and specular on top of NFS fragment colour
        vec3 unitNormal = normalize(surfaceNormal);
        vec3 unitVectorToCamera = normalize(toCameraVector);

        vec3 totalDiffuse = vec3(0.0f);
        vec3 totalSpecular = vec3(0.0f);

        for (int i = 0; i < MAX_TRACK_CONTRIB_LIGHTS; ++i){
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
        totalDiffuse = max(totalDiffuse, ambientFactor);// Min brightness

        float shadow = ShadowCalculation(lightSpace);
        vec3 ambient =  0.4f * nfsColor.rgb;
        vec3 lighting = (ambient + (1.0 - shadow) * (totalDiffuse + totalSpecular)) * nfsColor.rgb;
        color = vec4(lighting, 1.0);

        // Check if lighting is inside the spotlight cone
        float theta = dot(normalize(toSpotlightVector), normalize(-spotlightDirection));
        // Working with angles as cosines instead of degrees so a '>' is used.
        if (theta > spotlightCutOff) {
            color.rgb += (0.2 * spotlightColour.rgb);
        }
    }
}