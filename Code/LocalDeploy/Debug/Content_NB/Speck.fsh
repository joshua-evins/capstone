#version 400

in vec4 fragVel;
out vec4 FragColor;

void main() {
FragColor = vec4(0.5, 0.5, 0.5, 1.0) - (normalize(fragVel));
}