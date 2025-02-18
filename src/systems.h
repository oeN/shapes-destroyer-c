#pragma once

#include "entity.h"
#include <SDL3/SDL_render.h>

void moveSystem(entity_manager *em);
void renderPlayerSystem(entity_manager *em, SDL_Renderer *renderer);
void renderShapeSystem(game_context *gameContext, SDL_Renderer *renderer);
void keepInBoundsSystem(entity_manager *em);
void handlePlayerInput(linked_list_node *actionQueue, entity_manager *em);
