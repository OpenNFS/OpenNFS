in vec2 TexCoords;
out vec4 color;

uniform sampler2D textGlyphSampler;
uniform vec4 textColour;

void main() {
    float alpha = texture(textGlyphSampler, TexCoords).r;
    color = vec4(textColour.rgb, textColour.a * alpha);
}