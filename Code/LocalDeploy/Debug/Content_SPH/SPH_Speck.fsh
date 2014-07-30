#version 400

in float Lifetime;
out vec4 FragColor;

void main() {
//FragColor = mix(
//				vec4(1.0, 0.0, 0.0, 1.0),
//				vec4(0.0, 0.0, 1.0, 1.0),
//				smoothstep(0.0, 20.0, Lifetime)
//				);
float lifetimeNormalized = clamp(Lifetime/16.0, 0.0, 1.0);
FragColor = vec4(1.0-lifetimeNormalized, 0.0, lifetimeNormalized, 1.0);
}