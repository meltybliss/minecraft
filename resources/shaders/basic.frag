#version 330 core

in vec2 vUV;
in float vLight;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uFlash;
uniform float uAlpha;

void main() {
    vec4 tex = texture(uTexture, vUV);

    vec3 litColor = tex.rgb * vLight;
    vec3 color = mix(litColor, vec3(1.0), uFlash);

    FragColor = vec4(color, tex.a * uAlpha);
}