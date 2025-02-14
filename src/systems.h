#pragma once

#include <SDL3/SDL_render.h>
#include "entity.h"

void moveSystem(EntityManager *em);
void renderPlayerSystem(EntityManager *em, SDL_Renderer *renderer);
void renderShapeSystem(EntityManager *em, SDL_Renderer *renderer);
void keepInBoundsSystem(EntityManager *em);
void handlePlayerInput(EntityManager *em);
