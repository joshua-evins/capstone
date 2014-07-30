#ifndef MY_VERTEX_H
#define MY_VERTEX_H

struct vec3 {
  float x;
  float y;
  float z;
};

struct MyVertex
{
	float x, y, z;
	vec3 normal;
	unsigned int color;
	float u, v;
	uint skeletonIDs;
	float w1,w2,w3;
	MyVertex(float x = 0, float y = 0,
		float z = 0, float u = 0, float v = 0,
		unsigned int color = 0xFFFFFFFF) : 
		x(x), y(y), z(z),
		u(u), v(v) {
		normal.x = normal.y = normal.z = 0;
	  this->color = color;
	}
};

inline unsigned int ColorToInt(
	unsigned int r,
	unsigned int g,
	unsigned int b,
	unsigned int a = 0xFF)
{
	unsigned int ret = 0;
	ret |= r << 24;
	ret |= g << 16;
	ret |= b << 8;
	ret |= a << 0;
	return ret;
}

#endif