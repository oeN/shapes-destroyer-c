#pragma once
#include "components.h"

#define PI 3.141592f

#define degToRad(angleInDegrees) ((angleInDegrees) * PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / PI)

Vec2 vec2FromAngle(float angle);
Vec2 vec2Splat(float value);

void addToVec2(Vec2 *lhs, const Vec2 *rhs);
void extendVec2(Vec2 *lhs, float distance);
