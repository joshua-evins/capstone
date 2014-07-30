#version 400

layout (location=0) in vec3 position;
//layout (location=1) in vec4 vertexColor;

out vec4 color;

uniform mat4 transform;

void main() {
	color = vec4(0.3,1.0,0.3,1);
	gl_Position = transform * vec4(position, 1);
}