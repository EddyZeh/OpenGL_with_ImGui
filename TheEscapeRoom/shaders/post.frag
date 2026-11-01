#version 330 core
/*
    Escape Room post-processing (pairs with the engine's FBO.vert, driven by the engine's PostProcessor).
    Vignette, subtle film grain, a fade to black and a colour flash for feedback.
*/
in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float screenWidth;
uniform float screenHeight;
uniform int effect;             // set by PostProcessor; unused here

uniform float time;
uniform float vignette;         // 0 = none
uniform float grain;            // 0 = none
uniform float fade;             // 0 = clear, 1 = black
uniform vec3 flashColor;
uniform float flash;            // 0 .. 1

float hash(vec2 p){
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main(){
    vec3 color = texture(screenTexture, texCoords).rgb;

    // vignette (aspect corrected)
    vec2 uv = texCoords - 0.5;
    uv.x *= screenWidth / max(screenHeight, 1.0);
    float v = smoothstep(0.35, 1.05, length(uv));
    color *= 1.0 - v * vignette;

    // film grain
    float n = hash(texCoords * vec2(screenWidth, screenHeight) + fract(time) * 100.0) - 0.5;
    color += n * grain;

    color = mix(color, flashColor, flash);
    color *= 1.0 - fade;

    FragColor = vec4(color, 1.0);
}
