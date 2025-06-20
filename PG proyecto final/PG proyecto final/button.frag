#version 330 core
out vec4 FragColor;

uniform vec3 buttonColor;

void main() {
    FragColor = vec4(buttonColor, 1.0);
}