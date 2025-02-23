#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>

#include "constants.h"
#include "entity.h"
#include "game_context.h"
#include "memory.h"
#include "mymath.h"
#include "systems.h"
#include "types.h"

void spawnEntities(system_params *params) {
  if (!params->gameContext->entityManager)
    return;

  entity_manager *em = params->gameContext->entityManager;

  // for now the first entity is always the player and the other are all "ENEMY"
  // even if the currently do nothing
  for (int i = 0; i < 30; i++) {
    spawnEntity(em, true);
  }
}

void renderPlayerSystem(system_params *params) {
  if (!params->renderer)
    return;
  if (!params->gameContext)
    return;
  if (!params->gameContext->entityManager)
    return;

  entity_manager *em = params->gameContext->entityManager;
  SDL_Renderer *renderer = params->renderer;

  entity *e = getPlayer(em);
  if (!e)
    return;

  if (e->totalComponents <= 0)
    return;

  const Position *pos = getComponentValue(em, e, (component_name) "Position");
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

void renderShapeSystem(system_params *params) {
  if (!params->gameContext)
    return;
  if (!params->renderer)
    return;
  if (!params->tempArena)
    return;

  game_context *gameContext = params->gameContext;
  SDL_Renderer *renderer = params->renderer;

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

    // FIXME: all the SDL related function should be moved outside the game
    // "logic"
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

    float unitAngle = 360.0f / shape->pointCount;

    vec2 *points =
        pushSizeTimes(params->tempArena, vec2, shape->pointCount + 1);
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

      /*if (prev) {*/
      /*  SDL_RenderLine(renderer, prev->x, prev->y, current->x, current->y);*/
      /*}*/

      prev = current;
      currentAngle += unitAngle;
      pointsIndex++;
    }
    points[pointsIndex] = points[0];

    SDL_RenderLines(renderer, (SDL_FPoint *)points, shape->pointCount + 1);

    /*vec2 *lhs = prev;*/
    /*vec2 *rhs = &points[0];*/
    /**/
    /*SDL_RenderLine(renderer, lhs->x, lhs->y, rhs->x, rhs->y);*/
  }
}

void moveSystem(system_params *params) {
  if (!params->gameContext)
    return;

  if (!params->gameContext->entityManager)
    return;

  entity_manager *em = params->gameContext->entityManager;
  int current_position = 0;
  entity *e = 0;

  while (setEntityByTag(&e, em, ALL, current_position)) {
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

void keepInBoundsSystem(system_params *params) {
  if (!params->gameContext)
    return;

  entity_manager *em = getEntityManager(params->gameContext);
  if (!em)
    return;

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

void handlePlayerInput(system_params *params) {
  if (!params->gameContext)
    return;
  if (!params->gameContext->entityManager)
    return;
  if (!params->currentScene)
    return;

  entity_manager *em = params->gameContext->entityManager;
  linked_list_node *actionQueue = params->currentScene->actionsQueue;

  entity *player = getPlayer(em);
  if (!player)
    return;

  if (!actionQueue)
    return;

  Velocity *vel = getComponentValue(em, player, "Velocity");
  if (!vel)
    return;

  float acceleration = 5.0;

  // for now I'll reset the velocity
  // vel->x = 0.0;
  // vel->y = 0.0;

  u32 count = 0;
  linked_list_node *current = popFromLinkedList(actionQueue);
  // TODO: handle the first action itself
  // actually we cannot handle actions from the bottom, we've to handle them
  // from the start and remove the value as we go or "Pop from the front"
  while (current) {
    if (!current->value)
      continue;
    /*action *currentAction = getNodeValue(current, action);*/
    action *currentAction = (action *)current->value;

    switch (currentAction->kind) {
    case ACTION_UP:
      if (currentAction->state == ACTION_STATE_START)
        vel->y -= acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->y += acceleration;

      break;

    case ACTION_DOWN:
      if (currentAction->state == ACTION_STATE_START)
        vel->y += acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->y -= acceleration;

      break;

    case ACTION_LEFT:
      if (currentAction->state == ACTION_STATE_START)
        vel->x -= acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->x += acceleration;

      break;

    case ACTION_RIGHT:
      if (currentAction->state == ACTION_STATE_START)
        vel->x += acceleration;
      if (currentAction->state == ACTION_STATE_STOP)
        vel->x -= acceleration;

      break;

    default:
      break;
    }

    current = popFromLinkedList(actionQueue);
  }
}
