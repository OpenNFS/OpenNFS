in vec2 TexCoords;
out vec4 color;

uniform sampler2D textGlyphSampler;
uniform vec3 textColour;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textGlyphSampler, TexCoords).r);
    color = vec4(textColour, 1.0) * sampled;
}