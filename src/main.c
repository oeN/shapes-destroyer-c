#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "base.h"
#include "entity.h"
#include "types.h"
#include "math.h"

#define STEP_RATE_IN_MILLISECONDS 16.666667
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720


typedef struct AppState
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  EntityManager *entityManager;
  Uint64 last_step;
} AppState;

void moveSystem(EntityManager *em);
void renderRectSystem(EntityManager *em, AppState *as);
void renderShapeSystem(EntityManager *em, AppState *as);
void keepInBoundsSystem(EntityManager *em);
void handlePlayerInput(EntityManager *em);

enum Actions
{
  ACTION_UP,
  ACTION_DOWN,
  ACTION_LEFT,
  ACTION_RIGHT,
};

ActionMap defaultActions[] = {
  { .keycode = SDL_SCANCODE_W, .action = ACTION_UP },
  { .keycode = SDL_SCANCODE_S, .action = ACTION_DOWN },
  { .keycode = SDL_SCANCODE_D, .action = ACTION_RIGHT },
  { .keycode = SDL_SCANCODE_A, .action = ACTION_LEFT },
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
  srand(time(NULL)); // use current time as seed for random generator

  if (!SDL_Init(SDL_INIT_VIDEO))
    return SDL_APP_FAILURE;

  AppState *as = SDL_calloc(1, sizeof(AppState));
  if (!as)
    return SDL_APP_FAILURE;

  *appstate = as;
  as->entityManager = SDL_calloc(1, sizeof(EntityManager));
  as->last_step = SDL_GetTicks();

  // spawn a single entity
  for (int i = 0; i < 300; i++) {
    spawnEntity(as->entityManager, true);
  }

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &as->window, &as->renderer))
    return SDL_APP_FAILURE;

  return SDL_APP_CONTINUE;
}

Action *findAction(SDL_Scancode key_code)
{
  Action *found = SDL_calloc(1, sizeof(Action));

  for (int i =0; i < sizeof(defaultActions)/sizeof(defaultActions[0]); i++)
  {
    if (defaultActions[i].keycode == key_code) {
      found->action = defaultActions[i].action;
      break;
    }
  }

  return found;
}

SDL_AppResult handle_key_event(EntityManager *em, SDL_Event *event)
{
  switch (event->key.scancode)
  {
  case SDL_SCANCODE_Q:
  case SDL_SCANCODE_ESCAPE:
    return SDL_APP_SUCCESS;
  default:
  {
    Action *action = findAction(event->key.scancode);
    if (!action->action) break;

    action->state = event->type;
    addComponentToCurrentPlayer(em, "Action", action);
    break;
  }
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  AppState *as = appstate;
  switch (event->type)
  {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    return handle_key_event(as->entityManager, event);
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  AppState *as = (AppState *)appstate;
  const Uint64 now = SDL_GetTicks();

  while ((now - as->last_step) >= STEP_RATE_IN_MILLISECONDS)
  {
    moveSystem(as->entityManager);
    keepInBoundsSystem(as->entityManager);
    as->last_step += STEP_RATE_IN_MILLISECONDS;
  }

  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 255);
  SDL_RenderClear(as->renderer);

  /*renderRectSystem(as->entityManager, as);*/
  renderShapeSystem(as->entityManager, as);

  SDL_RenderPresent(as->renderer);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
  if (appstate != NULL)
  {
    AppState *as = (AppState *)appstate;
    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);
    for (int i = 0; i < as->entityManager->totalEntities; i++)
    {
      for (int c = 0; c < as->entityManager->entities[i]->totalComponents; c++)
      {
        SDL_free(as->entityManager->components[i][c]->value);
        SDL_free(as->entityManager->components[i][c]);
      }
      SDL_free(as->entityManager->entities[i]);
      SDL_free(as->entityManager->components[i]);
    }
    SDL_free(as->entityManager->entities);
    SDL_free(as->entityManager->components);
    SDL_free(as->entityManager);
    SDL_free(as);
  }
}

void moveSystem(EntityManager *em)
{
  Entity **entities = em->entities;
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = entities[i];

    if (e->totalComponents <= 0)
      continue;

    Position *pos = findComponent(em, e, "Position");
    if (!pos)
      continue;

    const Position *vel = findComponent(em, e, "Velocity");
    if (!vel)
      continue;

    pos->x += vel->x;
    pos->y += vel->y;
  }
}

void renderRectSystem(EntityManager *em, AppState *as)
{
  Entity **entities = em->entities;
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = entities[i];

    if (e->totalComponents <= 0)
      continue;


    const Position *pos = findComponent(em, e, "Position");
    if (!pos)
      continue;

    const Position *vel = findComponent(em, e, "Velocity");
    if (!vel)
      continue;

    SDL_FRect r = {
        .x = pos->x,
        .y = pos->y,
        .w = 50.0,
        .h = 50.0,
    };

    const Color *color = findComponent(em, e, "Color");
    if (!color)
      continue;

    SDL_SetRenderDrawColor(as->renderer, color->r, color->g, color->b, color->a);
    SDL_RenderRect(as->renderer, &r);
  }
}

void keepInBoundsSystem(EntityManager *em)
{
  Entity **entities = em->entities;
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = entities[i];

    if (e->totalComponents <= 0)
      continue;


    Position *pos = findComponent(em, e, "Position");
    if (!pos)
      continue;

    Velocity *vel = findComponent(em, e, "Velocity");
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

  Action *action = findComponent(em, player, "Action");

  // TODO: code to perform the action

  /*removeComponent(em, ction, player->id);*/
}

void renderShapeSystem(EntityManager *em, AppState *as)
{
  Entity **entities = em->entities;
  for (int i = 0; i < em->totalEntities; ++i)
  {
    Entity *e = entities[i];

    if (e->totalComponents <= 0)
      continue;

    const Position *pos = findComponent(em, e, "Position");
    if (!pos)
      continue;

    const Velocity *vel = findComponent(em, e, "Velocity");
    if (!vel)
      continue;

    const Shape *shape = findComponent(em, e, "Shape");
    if (!shape)
      continue;

    const Color *color = findComponent(em, e, "Color");
    if (!color)
      continue;

    SDL_SetRenderDrawColor(as->renderer, color->r, color->g, color->b, color->a);

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
        SDL_RenderLine(as->renderer, lhs->x, lhs->y, rhs->x, rhs->y);
      }
      prev = current;
      current = current->next;
    }

    Vec2 *lhs = prev->value;
    Vec2 *rhs = first->value;

    SDL_RenderLine(as->renderer, lhs->x, lhs->y, rhs->x, rhs->y);
    freeList(first);
  }
}
