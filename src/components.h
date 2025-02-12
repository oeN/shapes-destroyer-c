#pragma once

typedef struct Vec2
{
  float x;
  float y;
} Vec2;
typedef Vec2 Position;
typedef Vec2 Velocity;

typedef struct Shape
{
  float radius;
  int pointCount;
} Shape;

typedef struct Component
{
  unsigned long hash;
  void *value;
} Component;

