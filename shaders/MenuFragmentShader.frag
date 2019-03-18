in vec2 TexCoords;
out vec4 color;

uniform sampler2D menuTextureSampler;
uniform vec3 textColour;

void main()
{
    vec4 sampled = texture(menuTextureSampler, TexCoords).rgba;
    color = sampled;//vec4(textColour, 1.0) * sampled;
}