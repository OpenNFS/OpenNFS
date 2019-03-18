layout (location = 0) in vec4 vertexPosition_modelspace; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * vec4(vertexPosition_modelspace.xy, 0.0, 1.0);
    TexCoords = vertexPosition_modelspace.zw;
}