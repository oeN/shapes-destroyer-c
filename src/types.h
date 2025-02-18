#pragma once
#include <SDL3/SDL_keycode.h>

#include "base_types.h"

typedef struct GameContext game_context;

typedef struct Node node;
struct Node {
  node *next;
  void *value;
};

typedef struct String {
  char *value;
  u8 size;
} string;

typedef struct Action {
  uint8 action;
  uint32 state;
} action;

typedef struct ActionMap {
  SDL_Keycode keycode;
  uint8 action;
} action_map;

typedef struct Scene {
  action_map actions;
  string name;
} scene;

typedef union Vec2 {
  struct {
    float x, y;
  };
  float v[2];
} vec2;

typedef union Vec4 {
  struct {
    float r, g, b, a;
  };
  float v[4];
} vec4;
