#include <math.h>

#include "mymath.h"

vec2 vec2Splat(float value) { return (vec2){value}; }

vec2 vec2FromAngle(float angle) {
  float radians = degToRad(angle);
  vec2 v2 = {cosf(radians), sinf(radians)};
  return v2;
}

void addToVec2(vec2 *lhs, const vec2 *rhs) {
  lhs->x += rhs->x;
  lhs->y += rhs->y;
}

void extendVec2(vec2 *lhs, float distance) {
  lhs->x *= distance;
  lhs->y *= distance;
}
