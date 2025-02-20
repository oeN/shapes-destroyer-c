#include <stdio.h>

#include "game_context.h"
#include "game_engine.h"
#include "memory.h"

void printDebugInfo(game_engine *gameEngine) {
  printf("------ START UPDATE -----\n");
  printf("Game Engine %p\n", &gameEngine);
  printf("Arena %p\n", &gameEngine->mainArena);
  printf("Systems %p - count %d\n", &gameEngine->systems,
         gameEngine->systemsCount);
  printf("------ END UPDATE -----\n");
}

system_params generateSystemParams(game_engine *gameEngine) {
  return (system_params){
      .gameContext = gameEngine->gameContext,                   //
      .renderer = NULL,                                         //
      .currentScene = getCurrentScene(gameEngine->gameContext), //
  };
}

void _update(game_engine *self) {
  for (int i = 0; i < self->systemsCount; i++) {
    system_t *s = self->systems[i];
    // find a better way to do this
    if (s->gameLoopStage != UPDATE)
      continue;

    system_params p = generateSystemParams(self);
    s->callback(&p);
  }
}

void _destroy(game_engine *self) {
  freeGameContext(self->gameContext);
  // the game engine itself is inside the mainArena so we can just free that
  freeArena(self->mainArena);
}

int _addSystem(game_engine *self, game_loop_stage stage,
               system_callback systemCallback) {
  if (!self->systems)
    return 0;

  system_t *System = pushStruct(self->mainArena, system_t);
  System->gameLoopStage = stage;
  System->callback = systemCallback;
  self->systems[self->systemsCount++] = System;

  return 1;
}

game_engine *initGameEngine(memory_size mainArenaSize) {
  memory_arena *mainArena = bootstrapArena(mainArenaSize);

  game_engine *gameEngine = pushStruct(mainArena, game_engine);
  gameEngine->mainArena = mainArena;
  gameEngine->gameContext = initGameContext(mainArena);
  gameEngine->update = _update;
  gameEngine->destroy = _destroy;
  gameEngine->addSystem = _addSystem;
  gameEngine->systemsCount = 0;
  // TODO: find a solution to the limited number of systems
  gameEngine->systems = pushSizeTimes(gameEngine->mainArena, system_t *, 10);

  return gameEngine;
}
