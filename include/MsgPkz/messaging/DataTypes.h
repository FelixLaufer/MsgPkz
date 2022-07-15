#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

struct Vec3
{
  union
  {
    struct { float x; float y; float z; } vec;
    float raw[3];
  };

  Vec3() : raw() {}
  Vec3(const float x, const float y, const float z) : vec{ x, y, z } {}
};

struct Quat
{
  union
  {
    struct { float w; float x; float y; float z; } vec;
    float raw[4];
  };

  Quat() : raw() {}
  Quat(const float w, const float x, const float y, const float z) : vec{ w, x, y, z } {}
};

#endif