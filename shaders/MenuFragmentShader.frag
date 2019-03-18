in vec2 TexCoords;
out vec4 color;

uniform sampler2D menuTextureSampler;
uniform vec3 colour;

void main()
{
    vec4 sampled = texture(menuTextureSampler, TexCoords).rgba;
    color = vec4(colour, 1.0) * sampled;
}