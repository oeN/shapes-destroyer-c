#pragma once

#include "entity.h"
#include "memory.h"
#include "types.h"

struct GameContext {
  entity_manager *entityManager;
  memory_arena *frameArena;
  scene *currentScene;
};

game_context *initGameContext(memory_arena *gameArena);
void freeGameContext(game_context *gameContext);

entity_manager *getEntityManager(game_context *gameContext);
memory_arena *getFrameArena(game_context *gameContext);

scene *getCurrentScene(game_context *gameContext);
void setCurrentScene(game_context *gameContext, scene *setScene);
