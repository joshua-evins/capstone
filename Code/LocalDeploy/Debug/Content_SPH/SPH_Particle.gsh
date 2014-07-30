#version 400

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 transform;

out GSO
{
	flat vec4 center;
	vec4 position;
}gso;

in vec3 oX[];
in vec3 oZ[];

void main()
{
	
	vec3 qcenter = vec3(gl_in[0].gl_Position);
	vec4 gsoc = transform * vec4(qcenter, 1.0);
	
	vec4 pos = transform * vec4(qcenter + oX[0] + oZ[0], 1.0);
	gl_Position = vec4(pos);
	gso.position = vec4(pos);
	gso.center = vec4(gsoc);
	EmitVertex();
	
	pos = transform * vec4(qcenter + oX[0] - oZ[0], 1.0);
	gl_Position = vec4(pos);
	gso.position = vec4(pos);
	gso.center = vec4(gsoc);
	EmitVertex();
	
	pos = transform * vec4(qcenter - oX[0] + oZ[0], 1.0);
	gl_Position = vec4(pos);
	gso.position = vec4(pos);
	gso.center = vec4(gsoc);
	EmitVertex();
	
	pos = transform * vec4(qcenter - oX[0] - oZ[0], 1.0);
	gl_Position = vec4(pos);
	gso.position = vec4(pos);
	gso.center = vec4(gsoc);
	EmitVertex();
	
	EndPrimitive();
}