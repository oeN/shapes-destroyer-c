#pragma once

#include "entity.h"
#include "memory.h"
#include "types.h"

struct GameContext {
  entity_manager *entityManager;
  scene *currentScene;
};

game_context *initGameContext(memory_arena *gameArena);

entity_manager *getEntityManager(game_context *gameContext);

scene *getCurrentScene(game_context *gameContext);
void setCurrentScene(game_context *gameContext, scene *setScene);
