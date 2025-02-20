#include "game_context.h"
#include "entity.h"
#include "memory.h"
#include <SDL3/SDL_stdinc.h>

game_context *initGameContext(memory_arena *gameArena) {
  game_context *gc = pushStruct(gameArena, game_context);
  gc->frameArena = bootstrapArena(Kilobytes(500));
  gc->entityManager = pushStruct(gameArena, entity_manager);
  entity_manager *em = gc->entityManager;
  em->gameArena = gameArena;
  EntityManager_init(gc->entityManager);
  return gc;
}

scene *getCurrentScene(game_context *gameContext) {
  return gameContext->currentScene;
}

void setCurrentScene(game_context *gameContext, scene *setScene) {
  gameContext->currentScene = setScene;
}

entity_manager *getEntityManager(game_context *gameContext) {
  return gameContext->entityManager;
}

memory_arena *getFrameArena(game_context *gameContext) {
  return gameContext->frameArena;
}

void freeGameContext(game_context *gameContext) {
  if (!gameContext->frameArena)
    return;

  if (gameContext->frameArena->used == 0)
    return;

  freeArena(gameContext->frameArena);
}
