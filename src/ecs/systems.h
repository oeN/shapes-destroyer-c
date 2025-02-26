#pragma once

#include "../game_engine/types.h"
#include "../memory.h"
#include "entity.h"

// not all system uses all of the parameters, so any of them can be NULL
// each system should null check before using them
typedef struct SystemParams {
  entity_manager *entityManager;
  game_offscreen_buffer *backBuffer;
  memory_arena *tempArena;
} system_params;

typedef void (*system_callback)(system_params *);
typedef struct System {
  u8 gameLoopStage; // when the system must be called
  system_callback callback;
} system_t;

void spawnEntities(system_params *params);

void moveSystem(system_params *params);
void renderPlayerSystem(system_params *params);
void renderWeirdGradient(system_params *params);
void renderShapeSystem(system_params *params);
void keepInBoundsSystem(system_params *params);

void handlePlayerInput(system_params *params);
