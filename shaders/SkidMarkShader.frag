// Input from vertex shader
in vec2 UV;
in vec3 normal;
in float age;
in float intensity;
flat in uint surfaceType;

// Output color
out vec4 fragColor;

// Uniforms
uniform sampler2D skidTexture;
uniform float lifetime;      // Total lifetime in seconds (e.g., 15.0)
uniform float fadeStart;     // When fade begins (fraction of lifetime, e.g., 0.3)

// Surface type colors for different terrains
// 0=ASPHALT, 1=DIRT, 2=GRASS, 3=GRAVEL, 4=SNOW, 5=SAND, 6=UNKNOWN
const vec3 surfaceColors[7] = vec3[7](
    vec3(0.1, 0.1, 0.1),     // ASPHALT - dark rubber marks
    vec3(0.4, 0.25, 0.15),   // DIRT - brown
    vec3(0.2, 0.35, 0.1),    // GRASS - green-brown
    vec3(0.45, 0.4, 0.35),   // GRAVEL - gray-brown
    vec3(0.9, 0.95, 1.0),    // SNOW - white-blue
    vec3(0.76, 0.7, 0.5),    // SAND - tan
    vec3(0.3, 0.3, 0.3)      // UNKNOWN - gray
);

void main() {
    // Sample the skid mark texture
    vec4 texColor = texture(skidTexture, UV);

    // Get the surface-specific tint
    uint surfaceIdx = min(surfaceType, 6u);
    vec3 surfaceTint = surfaceColors[surfaceIdx];

    // Calculate age-based fade
    float normalizedAge = age / lifetime;
    float fadeAlpha = 1.0;

    if (normalizedAge > fadeStart) {
        // Linear fade from fadeStart to 1.0
        fadeAlpha = 1.0 - ((normalizedAge - fadeStart) / (1.0 - fadeStart));
    }

    // Clamp fade alpha
    fadeAlpha = clamp(fadeAlpha, 0.0, 1.0);

    // Combine intensity with fade
    float finalAlpha = texColor.a * intensity * fadeAlpha;

    // Apply surface tint to texture color
    vec3 finalColor = texColor.rgb * surfaceTint;

    // Discard fully transparent fragments
    if (finalAlpha < 0.01) {
        discard;
    }

    fragColor = vec4(finalColor, finalAlpha);
}
