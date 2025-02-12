#pragma once

#include <stdint.h>
typedef struct Component
{
  unsigned long hash;
  void *value;
} Component;

typedef struct Vec2
{
  float x;
  float y;
} Vec2;
typedef Vec2 Position;
typedef Vec2 Velocity;

typedef union Vec4
{ 
  struct {
    float r, g, b, a;
  };
  float v[4];
} Vec4;
typedef Vec4 Color;

typedef struct Shape
{
  float radius;
  int pointCount;
} Shape;

typedef struct Lifetime
{
  uint64_t timeToLive;
  uint64_t remaininigTimeToLive;
} Lifetime;

// TODO: add a type that could hold the 4 values of a color (r,g,b,a)
// TODO: add a color component
// TODO: add the color component to each rect
// TODO: use the color component to render the rect

enum ActionState 
{
  START,
  STOP
};


