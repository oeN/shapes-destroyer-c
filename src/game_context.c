#include "entity.h"
#include "memory.h"
#include <SDL3/SDL_stdinc.h>

struct GameContext {
  entity_manager entityManager;
  memory_arena frameArena;
  scene *currentScene;
};

game_context *initGameContext(memory_arena *gameArena) {
  game_context *gc = pushStruct(gameArena, game_context);
  gc->entityManager = (entity_manager){0};
  entity_manager *em = &gc->entityManager;
  em->gameArena = gameArena;
  EntityManager_init(&gc->entityManager);
  return gc;
}

scene *getCurrentScene(game_context *gameContext) {
  return gameContext->currentScene;
}

void setCurrentScene(game_context *gameContext, scene *setScene) {
  gameContext->currentScene = setScene;
}

entity_manager *getEntityManager(game_context *gameContext) {
  return &gameContext->entityManager;
}

memory_arena *getFrameArena(game_context *gameContext) {
  return &gameContext->frameArena;
}
