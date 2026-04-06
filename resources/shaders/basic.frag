#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uFlash;

void main() {
    vec4 tex = texture(uTexture, vUV);
    vec3 color = mix(tex.rgb, vec3(1.0), uFlash);
    FragColor = vec4(color, tex.a);
}