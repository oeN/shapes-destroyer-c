#pragma once

#include "memory.h"
#include "systems.h"

typedef struct GameEngine game_engine;

typedef enum GameLoopStage { INIT, UPDATE, RENDER } game_loop_stage;

struct GameEngine {
  memory_arena *mainArena;
  game_context *gameContext;

  u8 systemsCount;
  system_t **systems;

  void (*update)(game_engine *self);
  void (*destroy)(game_engine *self);
  int (*addSystem)(game_engine *self, game_loop_stage stage,
                   system_callback systemCallback);
};

system_params generateSystemParams(game_engine *gameEngine);

game_engine *initGameEngine(memory_size mainArenaSize);

// must be called in each loop
void _update(game_engine *self);

void _destroy(game_engine *self);
int _addSystem(game_engine *self, game_loop_stage stage,
               system_callback systemCallback);
