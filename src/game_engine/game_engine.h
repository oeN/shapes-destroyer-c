#pragma once

#include "../ecs/entity.h"
#include "../ecs/systems.h"
#include "../memory.h"
#include "types.h"

typedef struct GameEngine game_engine;

typedef enum GameLoopStage { //
  GAME_ENGINE_INIT,
  GAME_ENGINE_INPUT,
  GAME_ENGINE_UPDATE,
  GAME_ENGINE_RENDER
} game_loop_stage;

// for a single frame the context is always the same,
// like for the system_params
typedef struct frame_context {
  system_params *systemParams;
  memory_arena *frameArena;
} frame_context;

struct GameEngine {
  memory_arena *mainArena;
  entity_manager *entityManager;
  frame_context *frameContext;
  game_offscreen_buffer *backBuffer;

  u8 systemsCount;
  system_t **systems;
};

game_engine *bootstrapGameEngine(memory_size mainArenaSize);

void GameEngine_preFrame(game_engine *self);
void GameEngine_postFrame(game_engine *self);

void GameEngine_init(game_engine *self);
void GameEngine_update(game_engine *self);
void GameEngine_render(game_engine *self);

void GameEngine_destroy(game_engine *self);
int GameEngine_addSystem(game_engine *self, game_loop_stage stage,
                         system_callback systemCallback);
