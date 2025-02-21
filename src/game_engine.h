#pragma once

// FIXME: this is a temporary solution we don't want the game_engine to know or
// care about SDL
#include <SDL3/SDL.h>

#include "memory.h"
#include "systems.h"

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
  game_context *gameContext;
  frame_context *frameContext;
  // FIXME: as before, this is temporary we should have something to decouple
  // the rendering "platform"
  SDL_Renderer *renderer;

  u8 systemsCount;
  system_t **systems;

  void (*preFrame)(game_engine *self);
  void (*postFrame)(game_engine *self);
  void (*init)(game_engine *self);
  void (*update)(game_engine *self);
  void (*render)(game_engine *self);
  void (*destroy)(game_engine *self);
  int (*addSystem)(game_engine *self, game_loop_stage stage,
                   system_callback systemCallback);
};

game_engine *bootstrapGameEngine(memory_size mainArenaSize);

void _preFrame(game_engine *self);
void _postFrame(game_engine *self);

void _init(game_engine *self);
void _update(game_engine *self);
void _render(game_engine *self);

void _destroy(game_engine *self);
int _addSystem(game_engine *self, game_loop_stage stage,
               system_callback systemCallback);
