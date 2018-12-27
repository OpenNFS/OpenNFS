layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * transformationMatrix * vec4(vertexPosition_modelspace.xz, 0.0, 1.0);
    gl_Position.z = 0.0;
}