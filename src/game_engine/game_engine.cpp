#include "./game_engine.h"

#include <stdio.h>

#include "../base.h"
#include "../memory.h"

void printDebugInfo(wayne_t *gameEngine) {
  printf("------ START UPDATE -----\n");
  printf("Game Engine %p\n", &gameEngine);
  printf("Arena %p\n", &gameEngine->mainArena);
  printf("Systems %p - count %d\n", &gameEngine->Systems,
         gameEngine->SystemsCount);
  printf("------ END UPDATE -----\n");
}

void Wayne_preFrame(wayne_t *self) {
  frame_context *ctx = self->frameContext;
  resetArena(ctx->frameArena, false);

  ctx->systemParams = pushStruct(ctx->frameArena, system_params);
  ctx->systemParams->GameEngine = self;
  ctx->systemParams->entityManager = self->entityManager;
  ctx->systemParams->backBuffer = self->BackBuffer;
  ctx->systemParams->tempArena = ctx->frameArena;
  ctx->systemParams->AudioBuffer = self->AudioBuffer;
}

void Wayne_postFrame(wayne_t *self) {}

void loopThroughSystems(wayne_t *self, wayne_loop_stage stage) {
  for (int i = 0; i < self->SystemsCount; i++) {
    system_t *s = self->Systems[i];
    // find a better way to do this
    if (s->gameLoopStage != stage)
      continue;

    s->callback(self->frameContext->systemParams);
  }
}

void Wayne_init(wayne_t *self, u64 msFromStart) {
  self->msFromStart = msFromStart;
  // This is temporary usually the systems area added by an external piece of
  // code this file should contain only the basic pieces of the engine

  // INIT
  /*GameEngine_addSystem(self, GAME_ENGINE_INIT, spawnEntities);*/
  /**/
  /*// INPUT*/
  /*GameEngine_addSystem(self, GAME_ENGINE_INPUT, handlePlayerInput);*/
  /**/

  // UPDATE
  // Wayne_addSystem(self, WAYNE_UPDATE, generateAudio);
  // GameEngine_addSystem(self, WAYNE_UPDATE, moveSystem);
  /*GameEngine_addSystem(self, GAME_ENGINE_UPDATE, keepInBoundsSystem);*/

  // RENDER
  Wayne_addSystem(self, WAYNE_RENDER, renderWeirdGradient);
  /*GameEngine_addSystem(self, GAME_ENGINE_RENDER, renderShapeSystem);*/
  /*GameEngine_addSystem(self, GAME_ENGINE_RENDER, renderPlayerSystem);*/

  // this function is only called once but I don't like to trick the pre and
  // post frame in theory I need this only for the system params but if some
  // other system will need a temporary memory to do things is good to call them
  // for now
  //
  // FIXME: later when you're almost done with the project check if there is a
  // better way to handle this
  Wayne_preFrame(self);
  loopThroughSystems(self, WAYNE_INIT);
  Wayne_postFrame(self);
}

void Wayne_updateAndRender(wayne_t *self, u64 msFromStart,
                           wayne_controller_input *Controllers) {
  self->msFromStart = msFromStart;
  // FIXME: I feel there is a better way to do this but I might be wrong check
  // it
  for (int ControllerIndex = 0; ControllerIndex < MAX_N_CONTROLLERS;
       ControllerIndex++) {
    self->Controllers[ControllerIndex] = Controllers[ControllerIndex];
  }
  loopThroughSystems(self, WAYNE_INPUT);
  // TODO: calc and use delta Time or run the update enough to keep the
  // framerate stable
  loopThroughSystems(self, WAYNE_UPDATE);
  loopThroughSystems(self, WAYNE_RENDER);
}

void Wayne_destroy(wayne_t *self) {
  // even if the frame arena is inside the frameContext that it's inside the
  // game engine struct itself we've bootstrapped a diffeerent arena for the
  // frameContext and we've to free it
  freeArena(self->frameContext->frameArena);
  // the game engine itself is inside the mainArena so we can just free that
  freeArena(self->mainArena);
}

int Wayne_addSystem(wayne_t *self, wayne_loop_stage stage,
                    system_callback systemCallback) {
  if (!self->Systems)
    return 0;

  system_t *System = pushStruct(self->mainArena, system_t);
  System->gameLoopStage = stage;
  System->callback = systemCallback;
  self->Systems[self->SystemsCount++] = System;

  return 1;
}

wayne_t *bootstrapWayne(memory_size mainArenaSize) {
  memory_arena *mainArena = bootstrapArena(mainArenaSize);

  wayne_t *gameEngine = pushStruct(mainArena, wayne_t);
  gameEngine->mainArena = mainArena;
  gameEngine->BackBuffer = pushStruct(mainArena, game_offscreen_buffer);
  gameEngine->entityManager = pushStruct(mainArena, entity_manager);
  EntityManager_init(gameEngine->entityManager);
  // FIXME: do we need this?
  gameEngine->entityManager->gameArena = mainArena;

  gameEngine->frameContext = pushStruct(mainArena, frame_context);
  gameEngine->frameContext->frameArena = bootstrapArena(Kilobytes(500));
  gameEngine->SystemsCount = 0;
  // TODO: find a solution to the limited number of systems
  gameEngine->Systems = pushSizeTimes(gameEngine->mainArena, system_t *, 10);

  return gameEngine;
}

wayne_controller_input Wayne_getController(wayne_t *GameEngine,
                                           int ControllerIndex) {
  Assert(ControllerIndex < ArrayCount(GameEngine->Controllers));
  return GameEngine->Controllers[ControllerIndex];
}
