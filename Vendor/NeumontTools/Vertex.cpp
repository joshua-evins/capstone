#include "Vertex.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;

uint Neumont::Vertex::POSITION_OFFSET = 0;
uint Neumont::Vertex::COLOR_OFFSET = sizeof(vec3);
uint Neumont::Vertex::NORMAL_OFFSET = sizeof(vec3) + sizeof(vec4);
uint Neumont::Vertex::UV_OFFSET = sizeof(vec3) + sizeof(vec4) + sizeof(vec3);
uint Neumont::Vertex::SKELETONID_OFFSET = sizeof(vec3) + sizeof(vec4) + sizeof(vec3) + sizeof(vec2);
uint Neumont::Vertex::WEIGHT_OFFSET = sizeof(vec3) + sizeof(vec4) + sizeof(vec3) + sizeof(vec2) + sizeof(uint);
uint Neumont::Vertex::STRIDE = sizeof(Vertex);
