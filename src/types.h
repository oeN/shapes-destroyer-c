#pragma once
#include <stdint.h>
#include <SDL3/SDL_keycode.h>

#include "components.h"
#include "entity.h"

typedef struct GameContext
{
  EntityManager entityManager;
} GameContext;

typedef struct Node
{
  struct Node *next;
  void *value;
} Node;

typedef struct String
{
  char *value;
  uint8_t size;
} String;

typedef struct Action
{
  int action;
  int state;
} Action;

typedef struct ActionMap
{
  SDL_Keycode keycode;
  int action;
} ActionMap;

typedef struct Scene 
{
  ActionMap *actions;
  String name;
} Scene;

