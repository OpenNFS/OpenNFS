// Ouput data
layout(location = 0) out float fragmentdepth;

// Discard pixels for depth buffer based on Alpha (This might nerf perf)
in vec2 UV;
flat in uint texIndex;
uniform sampler2DArray texture_array;

void main(){
    vec4 tempColor = texture(texture_array, vec3(UV, texIndex)).rgba;
    if (tempColor.a <= 0.5)
         discard;

    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;
}