#include <math.h>

#include "mymath.h"

inline vec2 vec2Splat(float value) { return (vec2){value}; }

vec2 vec2FromAngle(float angle) {
  float radians = degToRad(angle);
  vec2 v2 = {cosf(radians), sinf(radians)};
  return v2;
}

f32 vec2Lenght(vec2 v) { return sqrtf((v.x * v.x) + (v.y * v.y)); }

vec2 vec2Normalize(vec2 v) {
  vec2 Result = v;
  f32 Lenght = vec2Lenght(v);
  Result /= Lenght;
  return Result;
}

// these can be replaced or already have been by operator overload
void addToVec2(vec2 *lhs, const vec2 *rhs) {
  lhs->x += rhs->x;
  lhs->y += rhs->y;
}

vec2 addTwoVec2(const vec2 lhs, const vec2 rhs) {
  vec2 result = lhs;
  addToVec2(&result, &rhs);
  return result;
}

void extendVec2(vec2 *lhs, float distance) {
  lhs->x *= distance;
  lhs->y *= distance;
}
