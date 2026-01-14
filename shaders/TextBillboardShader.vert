layout (location = 0) in vec4 vertexPosition_modelspace; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 billboardPos; // World position of text
uniform float scale;

void main() {
    // Get camera right and up vectors from view matrix
    vec3 cameraRight = vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    vec3 cameraUp = vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

    // Calculate billboard vertex position in world space
    vec3 vertexPosition_worldspace = billboardPos
    + cameraRight * vertexPosition_modelspace.x * scale
    + cameraUp * vertexPosition_modelspace.y * scale;

    // Transform to clip space
    gl_Position = projectionMatrix * viewMatrix * vec4(vertexPosition_worldspace, 1.0);

    TexCoords = vertexPosition_modelspace.zw;
}