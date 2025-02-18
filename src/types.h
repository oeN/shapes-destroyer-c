#pragma once
#include <SDL3/SDL_keycode.h>

#include "base_types.h"

typedef struct GameContext game_context;
typedef struct Scene scene, *scene_ptr;

typedef enum ActionState {
  ACTION_STATE_NONE,
  ACTION_STATE_START,
  ACTION_STATE_STOP
} action_state;

typedef enum ActionKind {
  ACTION_NONE,
  ACTION_UP,
  ACTION_DOWN,
  ACTION_LEFT,
  ACTION_RIGHT,
} action_kind;

typedef struct String {
  char *value;
  u8 size;
} string;

typedef struct Action {
  action_kind kind;
  action_state state;
} action;

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
