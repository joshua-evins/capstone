#version 400

layout (location=0) in vec4 position;
layout (location=1) in float lifetime;

uniform mat4 transform;
out float Lifetime;

void main() {
	Lifetime = lifetime;
	gl_Position = transform * vec4(position.xyz, 1.0);
}