#version 400

in GSO
{
	flat vec4 center;
	vec4 position;
}gso;

out vec4 FragColor;

void main() {
float diff = distance(gso.center, gso.position);
	if(diff > 0.12)
	{
		discard;
	}
	
FragColor = vec4(diff*3.0, diff*3.0, 1.0, 1.0);
}