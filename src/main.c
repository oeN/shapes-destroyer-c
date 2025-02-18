#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_scancode.h>
#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "constants.h"
#include "entity.h"
#include "game_context.h"
#include "memory.h"
#include "systems.h"
#include "types.h"

typedef struct AppState {
  SDL_Window *window;
  SDL_Renderer *renderer;
  game_context *gameContext;
  memory_arena *gameArena;
  u64 last_step;
} AppState;

// TODO: move these into a scene when implemented
enum Actions {
  ACTION_UP,
  ACTION_DOWN,
  ACTION_LEFT,
  ACTION_RIGHT,
};

action_map defaultActions[] = {
    {.keycode = SDL_SCANCODE_W, .action = ACTION_UP},
    {.keycode = SDL_SCANCODE_S, .action = ACTION_DOWN},
    {.keycode = SDL_SCANCODE_D, .action = ACTION_RIGHT},
    {.keycode = SDL_SCANCODE_A, .action = ACTION_LEFT},
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  srand(time(NULL)); // use current time as seed for random generator

  if (!SDL_Init(SDL_INIT_VIDEO))
    return SDL_APP_FAILURE;

  AppState *as = SDL_calloc(1, sizeof(AppState));
  if (!as)
    return SDL_APP_FAILURE;
  // this seems so wrong!
  as->gameArena = SDL_calloc(1, sizeof(memory_arena));
  initArena(as->gameArena, Megabytes(10));

  *appstate = as;
  // FIXME: this is a mess, at least I don't like it
  as->gameContext = initGameContext(as->gameArena);
  as->last_step = SDL_GetTicks();
  entity_manager *em = getEntityManager(as->gameContext);
  em->totalEntities = 0;

  // spawn a single entity
  for (int i = 0; i < 3; i++) {
    spawnEntity(em, true);
  }
  addEntity(em, (new_entity_params){.tag = PLAYER});
  addEntity(em, (new_entity_params){.tag = PLAYER});

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH,
                                   SCREEN_HEIGHT, 0, &as->window,
                                   &as->renderer))
    return SDL_APP_FAILURE;

  return SDL_APP_CONTINUE;
}

action *findAction(AppState *as, SDL_Scancode key_code) {
  action *found = pushStruct(as->gameArena, action);

  for (int i = 0; i < sizeof(defaultActions) / sizeof(defaultActions[0]); i++) {
    if (defaultActions[i].keycode == key_code) {
      found->action = defaultActions[i].action;
      break;
    }
  }

  return found;
}

SDL_AppResult handle_key_event(AppState *as, SDL_Event *event) {
  switch (event->key.scancode) {
  case SDL_SCANCODE_Q:
  case SDL_SCANCODE_ESCAPE:
    return SDL_APP_SUCCESS;
  default: {
    action *action = findAction(as, event->key.scancode);
    if (!action->action)
      break;

    entity_manager *em = getEntityManager(as->gameContext);
    action->state = event->type;
    addComponentToCurrentPlayer(em, "Action", action);
    break;
  }
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState *as = appstate;
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    return handle_key_event(as, event);
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *as = (AppState *)appstate;
  const Uint64 now = SDL_GetTicks();
  memory_arena *frameArena = getFrameArena(as->gameContext);
  entity_manager *em = getEntityManager(as->gameContext);
  initArena(frameArena, 1024 * 500); // 500 KB

  while ((now - as->last_step) >= STEP_RATE_IN_MILLISECONDS) {
    moveSystem(em);
    keepInBoundsSystem(em);
    as->last_step += STEP_RATE_IN_MILLISECONDS;
  }

  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 255);
  SDL_RenderClear(as->renderer);

  renderPlayerSystem(em, as->renderer);
  renderShapeSystem(as->gameContext, as->renderer);

  SDL_RenderPresent(as->renderer);

  freeArena(frameArena);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate != NULL) {
    AppState *as = (AppState *)appstate;
    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);
    freeArena(as->gameArena);
    SDL_free(as);
  }
}
