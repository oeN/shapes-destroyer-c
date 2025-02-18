#pragma once

#include "types.h"

typedef struct Component {
  // FIXME: we should be able to calculate the hash, for each component, only
  // one time
  unsigned long hash;
  void *value;
} Component;

typedef vec2 Position;
typedef vec2 Velocity;

typedef vec4 Color;

typedef struct Shape {
  float radius;
  int pointCount;
} Shape;

typedef struct Lifetime {
  u64 timeToLive;
  u64 remaininigTimeToLive;
} Lifetime;
