#include "./game_engine.h"

#include <stdio.h>

#include "../base.h"
#include "../memory.h"

void printDebugInfo(wayne_t *gameEngine) {
  printf("------ START UPDATE -----\n");
  printf("Game Engine %p\n", &gameEngine);
  printf("Arena %p\n", &gameEngine->PermanentStorage);
  printf("Systems %p - count %d\n", &gameEngine->Systems,
         gameEngine->SystemsCount);
  printf("------ END UPDATE -----\n");
}

void Wayne_preFrame(wayne_t *self) {
  frame_context *ctx = self->frameContext;

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

int Wayne_addSystem(wayne_t *self, wayne_loop_stage stage,
                    system_callback systemCallback) {
  if (!self->Systems)
    return 0;

  system_t *System = pushStruct(self->PermanentStorage, system_t);
  System->gameLoopStage = stage;
  System->callback = systemCallback;
  self->Systems[self->SystemsCount++] = System;

  return 1;
}

game_state *Wayne_GetGameState(void *GameEngine) {
  return Wayne_GetGameState((wayne_t *)GameEngine);
}

game_state *Wayne_GetGameState(wayne_t *GameEngine) {
  return GameEngine->GameState;
}

wayne_controller_input Wayne_getController(wayne_t *GameEngine,
                                           int ControllerIndex) {
  Assert(ControllerIndex < ArrayCount(GameEngine->Controllers));
  return GameEngine->Controllers[ControllerIndex];
}

extern "C" WAYNE_BOOTSTRAP(Wayne_bootstrap) {
  wayne_t *gameEngine = pushStruct(PermanentStorage, wayne_t);
  gameEngine->PermanentStorage = PermanentStorage;
  gameEngine->BackBuffer = pushStruct(PermanentStorage, game_offscreen_buffer);
  gameEngine->GameState = pushStruct(PermanentStorage, game_state);
  gameEngine->entityManager = pushStruct(PermanentStorage, entity_manager);
  EntityManager_init(gameEngine->entityManager);
  // FIXME: do we need this?
  gameEngine->entityManager->gameArena = PermanentStorage;

  gameEngine->frameContext = pushStruct(PermanentStorage, frame_context);
  gameEngine->frameContext->frameArena = TransientStorage;
  gameEngine->SystemsCount = 0;

  // TODO: find a solution to the limited number of systems
  u8 MaxNumberOfSystems = 10;
  gameEngine->Systems = pushSizeTimes(gameEngine->PermanentStorage, system_t *,
                                      MaxNumberOfSystems);

  return gameEngine;
}

extern "C" WAYNE_INIT(Wayne_init) {
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
  //
  // Wayne_addSystem(self, WAYNE_INIT,
  //                generateAudio); // make a pass in the init too
  // Wayne_addSystem(self, WAYNE_UPDATE, generateAudio);
  //
  // Wayne_addSystem(self, WAYNE_UPDATE,moveSystem);

  /*GameEngine_addSystem(self, GAME_ENGINE_UPDATE, keepInBoundsSystem);*/

  // RENDER
  // Wayne_addSystem(self, WAYNE_RENDER, renderWeirdGradient);
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

  //  loopThroughSystems(self, WAYNE_INIT);
  generateAudio(self->frameContext->systemParams);

  Wayne_postFrame(self);
}

extern "C" WAYNE_UPDATE_AND_RENDER(Wayne_updateAndRender) {
  self->msFromStart = msFromStart;
  // FIXME: I feel there is a better way to do this but I might be wrong check
  // it
  for (int ControllerIndex = 0; ControllerIndex < MAX_N_CONTROLLERS;
       ControllerIndex++) {
    self->Controllers[ControllerIndex] = Controllers[ControllerIndex];
  }

  // loopThroughSystems(self, WAYNE_INPUT);

  // TODO: calc and use delta Time or run the update enough to keep the
  // framerate stable
  // loopThroughSystems(self, WAYNE_UPDATE);
  // loopThroughSystems(self, WAYNE_RENDER);

  // temporary test to assure a thing about hot reload
  Wayne_preFrame(self);
  renderWeirdGradient(self->frameContext->systemParams);
  Wayne_postFrame(self);
}

extern "C" WAYNE_DESTROY(Wayne_destroy) {
  // TODO: check if this is still nedeed since we've moved the memory handling
  // part in the platform layer for now this function does nothing, maybe in the
  // future it will be needed but for now could be deleted to simplify the code
}
