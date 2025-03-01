#pragma once

#include "../constants.h"
#include "../ecs/entity.h"
#include "../ecs/systems.h"
#include "../memory.h"
#include "types.h"

typedef struct GameEngine wayne_t;

typedef enum GameLoopStage { //
  WAYNE_INIT,
  WAYNE_INPUT,
  WAYNE_UPDATE,
  WAYNE_RENDER
} wayne_loop_stage;

// for a single frame the context is always the same,
// like for the system_params
typedef struct frame_context {
  system_params *systemParams;
  memory_arena *frameArena;
} frame_context;

struct GameEngine {
  u64 msFromStart;
  memory_arena *mainArena;
  entity_manager *entityManager;
  frame_context *frameContext;
  game_offscreen_buffer *BackBuffer;
  wayne_audio_buffer *AudioBuffer;
  wayne_controller_input Controllers[MAX_N_CONTROLLERS];

  u8 SystemsCount;
  system_t **Systems;
};

wayne_t *bootstrapWayne(memory_size mainArenaSize);

void Wayne_preFrame(wayne_t *self);
void Wayne_postFrame(wayne_t *self);

void Wayne_init(wayne_t *self, u64 msFromStart);
void Wayne_updateAndRender(
    wayne_t *self, u64 msFromStart,
    wayne_controller_input Controllers[MAX_N_CONTROLLERS]);

void Wayne_destroy(wayne_t *self);
int Wayne_addSystem(wayne_t *self, wayne_loop_stage stage,
                    system_callback systemCallback);
wayne_controller_input Wayne_getController(wayne_t *GameEngine,
                                           int ControllerIndex);
