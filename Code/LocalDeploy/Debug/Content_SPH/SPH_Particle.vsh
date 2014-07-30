#version 400

layout (location=0) in vec4 position;

uniform vec3 camFacing;

out vec3 oX;
out vec3 oZ;

void main() {
	vec3 right = cross(camFacing, vec3(0.0, 1.0, 0.0));
	vec3 realUp = cross(right, camFacing);
	oX = normalize(right) * 0.005;
	oZ = normalize(realUp) * 0.005;
	vec3 varyingPos = position.xyz;

	gl_Position = vec4(varyingPos, 1);
}