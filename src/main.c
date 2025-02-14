#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
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
#include "systems.h"
#include "constants.h"


typedef struct AppState
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  GameContext gameContext;
  // TODO: remove me and use the one inside the gameContext
  EntityManager *entityManager;
  Uint64 last_step;
} AppState;


// TODO: move these into a scene when implemented
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
  as->gameContext = (GameContext){};
  EntityManager_init(&as->gameContext.entityManager);
  as->entityManager = &as->gameContext.entityManager;
  as->last_step = SDL_GetTicks();

  // spawn a single entity
  for (int i = 0; i < 3; i++) {
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

  renderPlayerSystem(as->entityManager, as->renderer);
  renderShapeSystem(as->entityManager, as->renderer);

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
    // I shouldn't do this, it must automated somehow
    for (int i = 0; i < as->entityManager->totalEntities; i++)
    {
      for (int c = 0; c < getEntity(as->entityManager, i)->totalComponents; c++)
      {
        SDL_free(as->entityManager->components[i][c].value);
      }
      SDL_free(as->entityManager->components[i]);
    }
    SDL_free(as->entityManager->entities);
    SDL_free(as);
  }
}


