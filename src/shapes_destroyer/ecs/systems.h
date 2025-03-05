#pragma once

#include "../../memory.h"
#include "../game_engine/types.h"
#include "entity.h"

// not all system uses all of the parameters, so any of them can be NULL
// each system should null check before using them
typedef struct SystemParams {
  entity_manager *entityManager;
  wayne_offscreen_buffer *backBuffer;
  wayne_audio_buffer *AudioBuffer;
  memory_arena *tempArena;
  // temporary add a reference to the game engine to test out if it is better to
  // create a struct with all the things we need or pass the game engine and
  // implement getter functions to retrieve what we need
  void *GameEngine;
} system_params;

typedef void (*system_callback)(system_params *);
typedef struct System {
  u8 gameLoopStage; // when the system must be called
  system_callback callback;
} system_t;

void generateAudio(system_params *params);
void spawnEntities(system_params *params);

void moveSystem(system_params *params);
void renderPlayerSystem(system_params *params);
void renderWeirdGradient(system_params *params);
void renderShapeSystem(system_params *params);
void keepInBoundsSystem(system_params *params);

void handlePlayerInput(system_params *params);
