#pragma once

#include "entity.h"
#include "memory.h"
#include "types.h"

game_context *initGameContext(memory_arena *gameArena);
entity_manager *getEntityManager(game_context *gameContext);
memory_arena *getFrameArena(game_context *gameContext);
