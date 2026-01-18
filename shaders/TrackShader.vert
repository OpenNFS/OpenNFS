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
out vec3 surfaceNormal;
// CSM
out vec3 fragPosWorldSpace;
out float clipSpaceZ;

// Values that stay constant for the whole mesh.
uniform mat4 projectionMatrix, viewMatrix, transformationMatrix;

void main(){
    vec4 worldPosition = transformationMatrix * vec4(vertexPosition_modelspace, 1.0);

    // CSM: Pass world position for cascade selection
    fragPosWorldSpace = worldPosition.xyz;

    // Pass through texture Index
    texIndex = textureIndex;

    // Pass through nfs shading data
    nfsDataOut = nfsData;

    // Pass through debug data
    debugDataOut = debugData;

    surfaceNormal = (transformationMatrix * vec4(normal, 0.0)).xyz;

    // Calculate clip space position for cascade selection
    vec4 viewSpacePos = viewMatrix * worldPosition;
    clipSpaceZ = -viewSpacePos.z;  // Positive view-space depth

	// Output position of the vertex, in clip space : MVP * position
    gl_Position = projectionMatrix * viewSpacePos;

    // UV of the vertex. No special space for this one.
	UV = vertexUV;
}
