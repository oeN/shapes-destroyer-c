#pragma once

#include "entity.h"
#include <SDL3/SDL_render.h>

// not all system uses all of the parameters, so any of them can be NULL
// each system should null check before using them
typedef struct SystemParams {
  game_context *gameContext;
  // it can be retrieved from the gameContext
  // entity_manager *entityManager;
  SDL_Renderer *renderer;
  // it holds the current actionQueue for the player input
  scene *currentScene;
} system_params;

typedef void (*system_callback)(system_params *);
typedef struct System {
  u8 gameLoopStage; // when the system must be called
  system_callback callback;
} system_t;

void moveSystem(system_params *params);
void renderPlayerSystem(system_params *params);
void renderShapeSystem(system_params *params);
void keepInBoundsSystem(system_params *params);

void handlePlayerInput(system_params *params);
