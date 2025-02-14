// Interpolated values from the vertex shaders
in vec2 envUV;
in vec2 UV;

// Diffuse and Specular
in vec3 surfaceNormal;
in vec3 toLightVector[MAX_CAR_CONTRIB_LIGHTS];
in vec3 toCameraVector;


flat in uint texIndex;
flat in uint polyFlag;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2DArray textureArray;
uniform sampler2D envMapTextureSampler;
uniform sampler2D carTextureSampler;

uniform vec4 lightColour[MAX_CAR_CONTRIB_LIGHTS];
uniform vec3 attenuation[MAX_CAR_CONTRIB_LIGHTS];

uniform vec4 carColour;
uniform vec4 carSecondaryColour;
uniform float shineDamper;
uniform float reflectivity;
uniform float envReflectivity;

uniform bool multiTextured;
uniform bool polyFlagged;

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
    totalDiffuse = max(totalDiffuse, 0.7f); // Min brightness

	// Output color = color of the texture at the specified UV
	color = vec4(totalDiffuse, 1.0) * (carTexColor + envReflectivity*envTexColor) + vec4(totalSpecular, 1.0);
}