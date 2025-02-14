#include <SDL3/SDL.h>

#include "entity.h"
#include "systems.h"
#include "types.h"
#include "math.h"
#include "base.h"
#include "constants.h"

void renderPlayerSystem(EntityManager *em, SDL_Renderer *renderer)
{
  Entity *e = getPlayer(em);
  if (!e) {
    SDL_Log("there is no player %p", e);
    return;
  }

  if (e->totalComponents <= 0) {
    /*SDL_Log("there is no components for the player %p", e);*/
    return;
  }

  const Position *pos = getComponentValue(em, e, "Position");
  if (!pos)
    return;

  const Velocity *vel = getComponentValue(em, e, "Velocity");
  if (!vel)
    return;

  const Shape *shape = getComponentValue(em, e, "Shape");
  if (!shape)
    return;

  const Color *color = getComponentValue(em, e, "Color");
  if (!color)
    return;

  /*SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);*/
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_FRect r = {
    .x = pos->x,
    .y = pos->y,
    .w = 70.0,
    .h = 70.0,
  };
  SDL_RenderRect(renderer, &r);
}

void renderShapeSystem(EntityManager *em, SDL_Renderer *renderer)
{
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = getEntity(em, i);

    if (e->totalComponents <= 0)
      continue;

    const Position *pos = getComponentValue(em, e, "Position");
    if (!pos)
      continue;

    const Velocity *vel = getComponentValue(em, e, "Velocity");
    if (!vel)
      continue;

    const Shape *shape = getComponentValue(em, e, "Shape");
    if (!shape)
      continue;

    const Color *color = getComponentValue(em, e, "Color");
    if (!color)
      continue;

    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

    float unitAngle = 360.0f/shape->pointCount;

    Node *first = SDL_calloc(1, sizeof(Node));
    Node *point = first;

    // TODO: probably this whole loop can be moved into a function, it could be useful
    // at least for this game to have a function that given an angle and a center returns
    // a list of vertices where to draw the lines
    for (float currentAngle = unitAngle; currentAngle <= 360.0f; currentAngle += unitAngle)
    {
      Vec2 *directionVector = SDL_calloc(1, sizeof(Vec2));
      *directionVector = vec2FromAngle(currentAngle);
      extendVec2(directionVector, shape->radius);
      addToVec2(directionVector, pos);
      point->value = directionVector;
      // TODO: move the node init to a function? could we use the struct trick with function pointers?
      point->next = SDL_calloc(1, sizeof(Node));
      point = point->next;
    }

    Node *current = first;
    Node *prev = NULL;
    while (current->next)
    {
      if (prev) {
        // draw the line
        Vec2 *lhs = prev->value;
        Vec2 *rhs = current->value;
        // TODO: move me to a function
        SDL_RenderLine(renderer, lhs->x, lhs->y, rhs->x, rhs->y);
      }
      prev = current;
      current = current->next;
    }

    Vec2 *lhs = prev->value;
    Vec2 *rhs = first->value;

    SDL_RenderLine(renderer, lhs->x, lhs->y, rhs->x, rhs->y);
    freeList(first);
  }
}

void moveSystem(EntityManager *em)
{
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = getEntity(em, i);

    if (e->totalComponents <= 0)
      continue;

    Position *pos = getComponentValue(em, e, "Position");
    if (!pos)
      continue;

    const Position *vel = getComponentValue(em, e, "Velocity");
    if (!vel)
      continue;

    pos->x += vel->x;
    pos->y += vel->y;
  }
}


void keepInBoundsSystem(EntityManager *em)
{
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = getEntity(em, i);

    if (e->totalComponents <= 0)
      continue;


    Position *pos = getComponentValue(em, e, "Position");
    if (!pos)
      continue;

    Velocity *vel = getComponentValue(em, e, "Velocity");
    if (!vel)
      continue;

    if (pos->x > SCREEN_WIDTH || pos->x < 0)
      vel->x *= -1;

    if (pos->y > SCREEN_HEIGHT || pos->y < 0)
      vel->y *= -1;
  }
}

void handlePlayerInput(EntityManager *em)
{
  Entity *player = getPlayer(em);
  if (!player) return;

  Action *action = getComponentValue(em, player, "Action");

  // TODO: code to perform the action

  /*removeComponent(em, ction, player->id);*/
}
