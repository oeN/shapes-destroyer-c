#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "base.h"
#include "entity.h"

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
void keepInBoundsSystem(EntityManager *em);

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
  int entityLimit = 100;
  for (int i = 0; i < entityLimit; i++) {
    spawnEntity(as->entityManager, true);
  }

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &as->window, &as->renderer))
    return SDL_APP_FAILURE;

  return SDL_APP_CONTINUE;
}

SDL_AppResult handle_key_event(SDL_Scancode key_code)
{
  switch (key_code)
  {
  case SDL_SCANCODE_Q:
  case SDL_SCANCODE_ESCAPE:
    return SDL_APP_SUCCESS;
  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  switch (event->type)
  {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    return handle_key_event(event->key.scancode);
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

  renderRectSystem(as->entityManager, as);

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

    Position *pos = findComponent(e, "Position");;
    if (!pos)
      continue;

    const Position *vel = findComponent(e, "Velocity");;
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


    const Position *pos = findComponent(e, "Position");;
    if (!pos)
      continue;

    const Position *vel = findComponent(e, "Velocity");;
    if (!vel)
      continue;

    SDL_FRect r = {
        .x = pos->x,
        .y = pos->y,
        .w = 50.0,
        .h = 50.0,
    };
    int red = 255;
    int green = 0;
    int blue = 0;
    SDL_SetRenderDrawColor(as->renderer, red, green, blue, 255);
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


    Position *pos = findComponent(e, "Position");
    if (!pos)
      continue;

    Velocity *vel = findComponent(e, "Velocity");
    if (!vel)
      continue;

    if (pos->x > SCREEN_WIDTH || pos->x < 0)
      vel->x *= -1;

    if (pos->y > SCREEN_HEIGHT || pos->y < 0)
      vel->y *= -1;
  }
}

