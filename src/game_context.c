#include "game_context.h"
#include "entity.h"
#include "memory.h"
#include <SDL3/SDL_stdinc.h>

game_context *initGameContext(memory_arena *gameArena) {
  game_context *gc = pushStruct(gameArena, game_context);
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
