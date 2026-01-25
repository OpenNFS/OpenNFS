// Input vertex data
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in float vertexAge;
layout(location = 4) in float vertexIntensity;
layout(location = 5) in uint vertexSurfaceType;

// Output to fragment shader
out vec2 UV;
out vec3 normal;
out float age;
out float intensity;
flat out uint surfaceType;

// Uniforms
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main() {
    // Pass through data to fragment shader
    UV = vertexUV;
    normal = vertexNormal;
    age = vertexAge;
    intensity = vertexIntensity;
    surfaceType = vertexSurfaceType;

    // Transform position to clip space
    gl_Position = projectionMatrix * viewMatrix * vec4(vertexPosition, 1.0);
}
