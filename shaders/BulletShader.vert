// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColour;

out vec3 vertColour;

// Values that stay constant for the whole mesh.
uniform mat4 projectionViewMatrix;

void main(){
    // Passthrough vertex colour
    vertColour = vertexColour;
	// Output position of the vertex, in clip space : MVP * position
    gl_Position =  projectionViewMatrix * vec4(vertexPosition_modelspace, 1.0f);
}

