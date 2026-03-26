#version 330 core

in vec2 aTexCoord;

out vec4 color;

uniform sampler2D textureSampler;

void main() {
    vec4 texColor = texture(textureSampler, aTexCoord.xy);

    color = texColor;
}