#ifndef ANIMATION_KEY_SET
#define ANIMATION_KEY_SET
#include <vector>
#include "glm\glm.hpp"

class TimeKey
{      
public:
  float time;
  glm::vec3 value;

  TimeKey()
  {       
    value.x = 0;
    value.y = 0;
    value.z = 0;
  }
};

class RotateKey
{
public:
  double time, value;
};


class TimeKeyFrame 
{
public:

  float time;
  glm::vec4 value;
  friend bool operator <  (const TimeKeyFrame& p1, const TimeKeyFrame& p2)
  {
    if (p1.time < p2.time)
      return true;
    return false;
  }
};


class AnimationKeyFrameSet
{
public:
  int index;
  //double timeStart, timeStop;
  std::vector<TimeKey> transKeys;
  std::vector<TimeKey> scaleKeys;
  std::vector<TimeKey> rotKeys;
  std::vector<TimeKeyFrame> quatKeys;

  void clear()
  {
    transKeys.clear();
    scaleKeys.clear();
    rotKeys.clear();
  }
};


inline float quatDot( const glm::vec4& b, const glm::vec4& a ) 
{
	return b.x * a.x + b.y * a.y + b.z * a.z + b.w * a.w;
};

class skeletonBone
{
public:
	char boneName[16];
	short ParentId;
};

#endif