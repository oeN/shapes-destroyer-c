#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>

#include "constants.h"
#include "entity.h"
#include "game_context.h"
#include "math.h"
#include "memory.h"
#include "systems.h"
#include "types.h"

void renderPlayerSystem(entity_manager *em, SDL_Renderer *renderer) {
  entity *e = getPlayer(em);
  if (!e)
    return;

  if (e->totalComponents <= 0)
    return;

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

void renderShapeSystem(game_context *gameContext, SDL_Renderer *renderer) {
  entity_manager *em = getEntityManager(gameContext);
  int current_position = 0;
  entity *e = 0;

  while (setEntityByTag(&e, em, ENEMY, current_position)) {
    // we already set the e variable we can increase the position immediately
    // and do not forget about it later
    current_position++;

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

    float unitAngle = 360.0f / shape->pointCount;

    vec2 *points =
        pushSizeTimes(getFrameArena(gameContext), vec2, shape->pointCount);
    int pointsIndex = 0;

    // TODO: probably this whole loop can be moved into a function, it could
    // be useful at least for this game to have a function that given an angle
    // and a center returns a list of vertices where to draw the lines
    float currentAngle = unitAngle;
    vec2 *prev = NULL;

    while (currentAngle <= 360.0f) {
      points[pointsIndex] = (vec2){};
      vec2 *current = &points[pointsIndex];

      { // angle vector goes out of scope immediately, find a better way?
        vec2 angleVector = vec2FromAngle(currentAngle);
        current->x = angleVector.x;
        current->y = angleVector.y;
      }

      extendVec2(current, shape->radius);
      addToVec2(current, pos);

      if (prev) {
        SDL_RenderLine(renderer, prev->x, prev->y, current->x, current->y);
      }

      prev = current;
      currentAngle += unitAngle;
      pointsIndex++;
    }

    vec2 *lhs = prev;
    vec2 *rhs = &points[0];

    SDL_RenderLine(renderer, lhs->x, lhs->y, rhs->x, rhs->y);
  }
}

void moveSystem(entity_manager *em) {
  int current_position = 0;
  entity *e = 0;

  while (setEntityByTag(&e, em, ENEMY, current_position)) {
    current_position++;

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

void keepInBoundsSystem(entity_manager *em) {
  for (int i = 0; i < em->totalEntities; ++i) {
    entity *e = getEntity(em, i);

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

void handlePlayerInput(entity_manager *em) {
  entity *player = getPlayer(em);
  if (!player)
    return;

  action *action = getComponentValue(em, player, "Action");

  // TODO: code to perform the action

  /*removeComponent(em, ction, player->id);*/
}
