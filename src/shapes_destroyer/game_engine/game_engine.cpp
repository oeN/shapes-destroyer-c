#include "./game_engine.h"
#include <stdio.h>

#include "../../base.h"
#include "../../memory.h"
#include "../../mymath.h"

void DrawRectangle(wayne_offscreen_buffer *Buffer, vec2 Position,
                   vec2 Dimensions, u32 Color) {

  vec2 Max = Position + Dimensions;

  i32 MinX = (i32)Clamp(f32, Position.x, 0, Buffer->Width);
  i32 MinY = (i32)Clamp(f32, Position.y, 0, Buffer->Height);
  i32 MaxX = (i32)Clamp(f32, Max.x, 0, Buffer->Width);
  i32 MaxY = (i32)Clamp(f32, Max.y, 0, Buffer->Height);

  uint8 *Row = ((uint8 *)Buffer->Memory + Buffer->Pitch * MinY +
                Buffer->BytesPerPixel * MinX);

  for (int Y = MinY; Y < MaxY; ++Y) {
    uint32 *Pixel = (uint32 *)Row;

    for (int X = MinX; X < MaxX; ++X) {
      *Pixel++ = Color;
    }

    Row += Buffer->Pitch;
  }
}

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
    system_t s = self->Systems[i];
    // find a better way to do this
    if (s.gameLoopStage != stage)
      continue;

    s.callback(self->frameContext->systemParams);
  }
}

int Wayne_addSystem(wayne_t *self, wayne_loop_stage stage,
                    system_callback systemCallback) {
  if (!self->Systems)
    return 0;

  system_t System = {0};
  System.gameLoopStage = stage;
  System.callback = systemCallback;
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

extern "C" WAYNE_RESET_SYSTEMS(Wayne_ResetSystems) {
#if USE_ECS_SYSTEMS
  // this is my temporary fix to the function pointers problem on the hot reload
  // feature I don't feel like is a good solution but it works for now and I'm
  // willing to keep it because it should make my life easier progressing
  // through the game and I can fix it later if I need to
  wayne_t *self = (wayne_t *)PermanentStorage->startAddress;
  self->SystemsCount = 0;

  Wayne_addSystem(self, WAYNE_RENDER, renderWeirdGradient);
#endif
}

extern "C" WAYNE_BOOTSTRAP(Wayne_bootstrap) {
  // 2025-03-04: right now it seems that I don't know how to this stuff
  // differently so for now I'll leave it as it is
  wayne_t *gameEngine = pushStruct(PermanentStorage, wayne_t);
  gameEngine->PermanentStorage = PermanentStorage;
  gameEngine->BackBuffer = pushStruct(PermanentStorage, wayne_offscreen_buffer);
  gameEngine->GameState = pushStruct(PermanentStorage, game_state);
  gameEngine->entityManager = pushStruct(PermanentStorage, entity_manager);
  EntityManager_init(gameEngine->entityManager);
  // FIXME: do we need this?
  gameEngine->entityManager->gameArena = PermanentStorage;

  gameEngine->frameContext = pushStruct(PermanentStorage, frame_context);
  gameEngine->frameContext->frameArena = TransientStorage;

  // TODO: find a solution to the limited number of systems
  gameEngine->SystemsCount = 0;
  gameEngine->MaxSystems = 10;
  gameEngine->Systems =
      pushSizeTimes(PermanentStorage, system_t, gameEngine->MaxSystems);

  return gameEngine;
}

// Move this code somewhere else
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

//  loopThroughSystems(self, WAYNE_INIT);
//

extern "C" WAYNE_UPDATE_AND_RENDER(Wayne_updateAndRender) {
  // 2025-03-04: ok this works because I know that the first thing I push in the
  // PermanentStorage is always the wayne_t struct itself, and currently it
  // seems smart but I feel like it shouldn't be done or it will bite me in the
  // future
  wayne_t *self = (wayne_t *)PermanentStorage->startAddress;

  self->msFromStart = msFromStart;
  self->AudioBuffer = AudioBuffer;
  self->BackBuffer = BackBuffer;

  // FIXME: I feel there is a better way to do this but I might be wrong check
  // it
  for (int ControllerIndex = 0; ControllerIndex < ArrayCount(self->Controllers);
       ControllerIndex++) {
    self->Controllers[ControllerIndex] = Controllers[ControllerIndex];
  }

  Wayne_preFrame(self);
  system_params *SystemParams = self->frameContext->systemParams;

#if USE_ECS_SYSTEMS
  loopThroughSystems(self, WAYNE_INPUT);
  loopThroughSystems(self, WAYNE_UPDATE);
  loopThroughSystems(self, WAYNE_RENDER);
#else
  // TODO: maybe leverage CPP function overload to have a more clear view of
  // wath each system requires without masking it behind the system_params
  // struct
  handlePlayerInput(SystemParams);
  DrawRectangle(
      self->BackBuffer, (vec2){.x = 0.0f, .y = 0.0f},
      (vec2){.w = self->BackBuffer->Width, .h = self->BackBuffer->Height},
      0xFF87ceeb);
  DrawRectangle(self->BackBuffer, self->GameState->PlayerPosition,
                (vec2){.w = 50.0f, .h = 50.0f}, 0xFFFFFFFF);
#endif

  Wayne_postFrame(self);
}

extern "C" WAYNE_DESTROY(Wayne_destroy) {
  // TODO: check if this is still nedeed since we've moved the memory handling
  // part in the platform layer for now this function does nothing, maybe in the
  // future it will be needed but for now could be deleted to simplify the code
}
