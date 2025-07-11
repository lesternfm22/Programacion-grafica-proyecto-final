#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec2 flippedTexCoords = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, flippedTexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}