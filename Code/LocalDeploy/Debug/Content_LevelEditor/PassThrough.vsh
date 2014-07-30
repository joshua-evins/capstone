#version 400

layout (location=0) in vec3 position;
layout (location=1) in vec4 vertexColor;

out vec4 color;

uniform mat4 transform;

void main() {
	color = vertexColor;
	gl_Position = transform * vec4(position, 1);
}