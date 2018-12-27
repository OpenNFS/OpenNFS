// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D boardTextureSampler;
uniform vec4 lightColour;

void main(){
    vec4 lightTexColour = texture( boardTextureSampler, UV ).rgba;

    if (lightTexColour.a == 0.0)
        discard;

	// Output color = color of the texture at the specified UV
	color = lightTexColour * lightColour;
} // note to self: needs more tho