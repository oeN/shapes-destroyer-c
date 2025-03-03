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

struct game_state {
  u16 BlueOffset;
  u16 GreenOffset;
};

struct GameEngine {
  u64 msFromStart;
  memory_arena *PermanentStorage;
  memory_arena *TransientStorage;
  entity_manager *entityManager;
  frame_context *frameContext;
  game_offscreen_buffer *BackBuffer;
  wayne_audio_buffer *AudioBuffer;
  game_state *GameState;

  wayne_controller_input Controllers[MAX_N_CONTROLLERS];

  u8 SystemsCount;
  system_t **Systems;
};

// FIXME: the bootstrap should take in the allocated memory itself and shouldn't
// do the allocation, that part must be done on the platform layer
// TODO: check if we still need the bootstrap after that
#define WAYNE_BOOTSTRAP(name)                                                  \
  wayne_t *(name)(memory_arena * PermanentStorage,                             \
                  memory_arena * TransientStorage)
typedef WAYNE_BOOTSTRAP(wayne_bootstrap);
WAYNE_BOOTSTRAP(Wayne_bootsrapStub) { return NULL; }

#define WAYNE_UPDATE_AND_RENDER(name)                                          \
  void(name)(wayne_t * self, u64 msFromStart,                                  \
             wayne_controller_input Controllers[MAX_N_CONTROLLERS])
typedef WAYNE_UPDATE_AND_RENDER(wayne_update_and_render);
WAYNE_UPDATE_AND_RENDER(Wayne_updateAndRenderStub) {}

#define WAYNE_INIT(name) void(name)(wayne_t * self, u64 msFromStart)
typedef WAYNE_INIT(wayne_init);
WAYNE_INIT(Wayne_initStub) {}

#define WAYNE_DESTROY(name) void(name)(wayne_t * self)
typedef WAYNE_DESTROY(wayne_destroy);
WAYNE_DESTROY(Wayne_destroyStub) {}

void Wayne_preFrame(wayne_t *self);
void Wayne_postFrame(wayne_t *self);

int Wayne_addSystem(wayne_t *self, wayne_loop_stage stage,
                    system_callback systemCallback);
wayne_controller_input Wayne_getController(wayne_t *GameEngine,
                                           int ControllerIndex);

game_state *Wayne_GetGameState(void *GameEngine);
game_state *Wayne_GetGameState(wayne_t *GameEngine);
