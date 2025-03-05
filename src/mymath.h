#pragma once
#include "types.h"

#define PI 3.141592f

#define degToRad(angleInDegrees) ((angleInDegrees) * PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / PI)

#define Clamp(type, value, min, max)                                           \
  (type)(value < min ? min : (value > max ? max : value))

vec2 vec2FromAngle(float angle);
vec2 vec2Splat(float value);

vec2 addTwoVec2(const vec2 lhs, const vec2 rhs);
void addToVec2(vec2 *lhs, const vec2 *rhs);
void extendVec2(vec2 *lhs, float distance);
