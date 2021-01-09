// Based on https://github.com/evanw/glfx.js/blob/master/src/filters/blur/zoomblur.js

#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    float dissolve;
    float strength;
    vec2 center;
};
layout(binding = 1) uniform sampler2D sourceTex;
layout(binding = 2) uniform sampler2D targetTex;

/* random number between 0 and 1 */
float random(in vec3 scale, in float seed) {
    /* use the fragment position for randomness */
    return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);
}

vec3 crossFade(in vec2 uv) {
    return mix(texture(sourceTex, uv).rgb,
               texture(targetTex, uv).rgb,
               dissolve);
}

void main() {
    vec3 color = vec3(0.0);
    float total = 0.0;
    vec2 toCenter = center - qt_TexCoord0;

    /* randomize the lookup values to hide the fixed number of samples */
    float offset = random(vec3(12.9898, 78.233, 151.7182), 0.0);

    for (float t = 0.0; t <= 40.0; t++) {
        float percent = (t + offset) / 40.0;
        float weight = 4.0 * (percent - percent * percent);
        color += crossFade(qt_TexCoord0 + toCenter * percent * strength) * weight;
        total += weight;
    }
    fragColor = vec4(color / total, 1.0) * qt_Opacity;
}