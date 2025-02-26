#include <stdio.h>

#include "./game_engine.h"
#include "memory.h"

void printDebugInfo(game_engine *gameEngine) {
  printf("------ START UPDATE -----\n");
  printf("Game Engine %p\n", &gameEngine);
  printf("Arena %p\n", &gameEngine->mainArena);
  printf("Systems %p - count %d\n", &gameEngine->systems,
         gameEngine->systemsCount);
  printf("------ END UPDATE -----\n");
}

void GameEngine_preFrame(game_engine *self) {
  frame_context *ctx = self->frameContext;
  resetArena(ctx->frameArena, false);

  ctx->systemParams = pushStruct(ctx->frameArena, system_params);
  ctx->systemParams->entityManager = self->entityManager;
  ctx->systemParams->backBuffer = self->backBuffer;
  ctx->systemParams->tempArena = ctx->frameArena;
}

void GameEngine_postFrame(game_engine *self) {}

void loopThroughSystems(game_engine *self, game_loop_stage stage) {
  for (int i = 0; i < self->systemsCount; i++) {
    system_t *s = self->systems[i];
    // find a better way to do this
    if (s->gameLoopStage != stage)
      continue;

    s->callback(self->frameContext->systemParams);
  }
}

void GameEngine_init(game_engine *self) {
  // INIT
  /*GameEngine_addSystem(self, GAME_ENGINE_INIT, spawnEntities);*/
  /**/
  /*// INPUT*/
  /*GameEngine_addSystem(self, GAME_ENGINE_INPUT, handlePlayerInput);*/
  /**/
  /*// UPDATE*/
  /*GameEngine_addSystem(self, GAME_ENGINE_UPDATE, moveSystem);*/
  /*GameEngine_addSystem(self, GAME_ENGINE_UPDATE, keepInBoundsSystem);*/

  // RENDER
  GameEngine_addSystem(self, GAME_ENGINE_RENDER, renderWeirdGradient);
  /*GameEngine_addSystem(self, GAME_ENGINE_RENDER, renderShapeSystem);*/
  /*GameEngine_addSystem(self, GAME_ENGINE_RENDER, renderPlayerSystem);*/

  // this function is only called once but I don't like to trick the pre and
  // post frame in theory I need this only for the system params but if some
  // other system will need a temporary memory to do things is good to call them
  // for now
  //
  // FIXME: later when you're almost done with the project check if there is a
  // better way to handle this
  GameEngine_preFrame(self);
  loopThroughSystems(self, GAME_ENGINE_INIT);
  GameEngine_postFrame(self);
}

void GameEngine_update(game_engine *self) {
  loopThroughSystems(self, GAME_ENGINE_INPUT);
  loopThroughSystems(self, GAME_ENGINE_UPDATE);
}

void GameEngine_render(game_engine *self) {
  loopThroughSystems(self, GAME_ENGINE_RENDER);
}

void GameEngine_destroy(game_engine *self) {
  // even if the frame arena is inside the frameContext that it's inside the
  // game engine struct itself we've bootstrapped a diffeerent arena for the
  // frameContext and we've to free it
  freeArena(self->frameContext->frameArena);
  // the game engine itself is inside the mainArena so we can just free that
  freeArena(self->mainArena);
}

int GameEngine_addSystem(game_engine *self, game_loop_stage stage,
                         system_callback systemCallback) {
  if (!self->systems)
    return 0;

  system_t *System = pushStruct(self->mainArena, system_t);
  System->gameLoopStage = stage;
  System->callback = systemCallback;
  self->systems[self->systemsCount++] = System;

  return 1;
}

game_engine *bootstrapGameEngine(memory_size mainArenaSize) {
  memory_arena *mainArena = bootstrapArena(mainArenaSize);

  game_engine *gameEngine = pushStruct(mainArena, game_engine);
  gameEngine->mainArena = mainArena;
  gameEngine->backBuffer = pushStruct(mainArena, game_offscreen_buffer);
  gameEngine->entityManager = pushStruct(mainArena, entity_manager);
  EntityManager_init(gameEngine->entityManager);
  // FIXME: do we need this?
  gameEngine->entityManager->gameArena = mainArena;

  gameEngine->frameContext = pushStruct(mainArena, frame_context);
  gameEngine->frameContext->frameArena = bootstrapArena(Kilobytes(500));
  gameEngine->systemsCount = 0;
  // TODO: find a solution to the limited number of systems
  gameEngine->systems = pushSizeTimes(gameEngine->mainArena, system_t *, 10);

  return gameEngine;
}
