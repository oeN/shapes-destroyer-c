#include "entity.h"
#include "memory.h"
#include <SDL3/SDL_stdinc.h>
#include <stdio.h>

struct GameContext {
  entity_manager entityManager;
  memory_arena frameArena;
};

game_context *initGameContext(memory_arena *gameArena) {
  game_context *gc = pushStruct(gameArena, game_context);
  gc->entityManager = (entity_manager){0};
  entity_manager *em = &gc->entityManager;
  em->gameArena = gameArena;
  EntityManager_init(&gc->entityManager);
  return gc;
}

entity_manager *getEntityManager(game_context *gameContext) {
  return &gameContext->entityManager;
}

memory_arena *getFrameArena(game_context *gameContext) {
  return &gameContext->frameArena;
}
