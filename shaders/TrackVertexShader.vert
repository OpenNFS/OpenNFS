// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 normal;
layout(location = 3) in uint textureIndex;
layout(location = 4) in vec4 nfsData;
layout(location = 5) in uint debugData;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
flat out uint texIndex;
out vec4 nfsDataOut;
flat out uint debugDataOut;

// Diffuse and Specular
out vec3 toLightVector[MAX_LIGHTS];
out vec3 toCameraVector;
out vec3 surfaceNormal;
// Fog
out vec3 worldPosition;
out vec4 viewSpace;
out vec4 lightSpace;

// Values that stay constant for the whole mesh.
uniform mat4 projectionMatrix, viewMatrix ,transformationMatrix;
uniform mat4 lightSpaceMatrix;
uniform vec3 lightPosition[MAX_LIGHTS];

void main(){
    vec4 worldPosition = transformationMatrix * vec4(vertexPosition_modelspace, 1.0);

    lightSpace = lightSpaceMatrix * worldPosition;

    // Pass through texture Index
    texIndex = textureIndex;

    // Pass through nfs shading data
    nfsDataOut = nfsData;

    // Pass through debug data
    debugDataOut = debugData;

    surfaceNormal = (transformationMatrix * vec4(normal, 0.0)).xyz;

    // Diffuse and Specular passout
    for(int i = 0; i < MAX_LIGHTS; ++i){
        toLightVector[i] = lightPosition[i] - worldPosition.xyz;
    }
    toCameraVector = (inverse(viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - worldPosition.xyz;

    // Fog Passout
    viewSpace = viewMatrix * transformationMatrix *  vec4(vertexPosition_modelspace, 1);

	// Output position of the vertex, in clip space : MVP * position
    gl_Position =  projectionMatrix * viewMatrix * worldPosition;
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}

