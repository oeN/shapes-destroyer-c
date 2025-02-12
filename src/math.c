#include <math.h>
#include "math.h"

Vec2 vec2Splat(float value)
{
  return (Vec2){value};
}

Vec2 vec2FromAngle(float angle)
{
  float radians = degToRad(angle);
  Vec2 v2 = {cosf(radians),sinf(radians)};
  return v2;
}

void addToVec2(Vec2 *lhs, const Vec2 *rhs)
{
  lhs->x += rhs->x;
  lhs->y += rhs->y;
}

void extendVec2(Vec2 *lhs, float distance)
{
  lhs->x *= distance;
  lhs->y *= distance;
}
