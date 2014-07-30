#version 400


layout (location=0) in vec3 position;
layout (location=1) in vec2 vertUV;

struct RippleEffect
{
	vec4 origin;
	float rippleDistance;
	int active;
	float padding[2];
};
uniform RippleEffects
{
	RippleEffect effects[50];
}ripples;

struct WaveEffect
{
	vec3 waveDir;
	float waveDistance;
	vec3 origin;
	bool active;
};
uniform WaveEffects
{
	WaveEffect effects[10];
}waves;

uniform mat4 transform;

void main() {
	vec3 varyingPos = position;
	
	for(int i=0; i<50; i++)
	{
		if(ripples.effects[i].active != 0)
		{
			float rippleDistance = ripples.effects[i].rippleDistance;
			float originToVert = distance(ripples.effects[i].origin.xyz, position);
			float distanceBehindRipple = rippleDistance - originToVert;
			float theta = distanceBehindRipple * 0.78539;
			theta = clamp(theta, 0.0, 6.28318);
			varyingPos.y += sin(theta*4)/(1.0+rippleDistance*0.6+(theta*3));
		}
	}
	
	for(int i=0; i<10; i++)
	{
		if(waves.effects[i].active)
		{
			float waveDistance = waves.effects[i].waveDistance;
			float distanceBehindWave = waveDistance - dot(position-waves.effects[i].origin, waves.effects[i].waveDir);
			if(distanceBehindWave < waveDistance)
			{
				float theta = distanceBehindWave * 0.78539;
				theta = clamp(theta, 0.0, min(waves.effects[i].waveDistance, 6.28318));
				varyingPos.y += sin(theta*4)/(1.0+waveDistance*0.6+(theta*3));
			}
		}
	}

	gl_Position = transform * vec4(varyingPos, 1);
}