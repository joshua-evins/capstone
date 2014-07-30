#version 400

layout (location=0) in vec4 position;
layout (location=1) in vec4 vertVel;

out vec4 fragVel;
uniform mat4 transform;

void main() {
	fragVel = vertVel;
	gl_Position = transform * vec4(position.xyz, 1.0);
}