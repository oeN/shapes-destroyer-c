#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
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
#include "game_engine.h"
#include "memory.h"
#include "systems.h"
#include "types.h"

typedef struct AppState {
  SDL_Window *window;
  SDL_Renderer *renderer;
  game_engine *gameEngine;
  u64 lastStep;
} AppState;

// these will be scene dependant
action_map defaultActions[] = {
    {.keycode = SDL_SCANCODE_W, .action = ACTION_UP},
    {.keycode = SDL_SCANCODE_S, .action = ACTION_DOWN},
    {.keycode = SDL_SCANCODE_D, .action = ACTION_RIGHT},
    {.keycode = SDL_SCANCODE_A, .action = ACTION_LEFT},
};

// these do not depend on a scene so we can hardcode them
action_state actionStateFromEventType(SDL_EventType eventType) {
  switch (eventType) {
  case SDL_EVENT_KEY_DOWN:
    return ACTION_STATE_START;
  case SDL_EVENT_KEY_UP:
    return ACTION_STATE_STOP;
  default:
    return ACTION_STATE_NONE;
  }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  srand(time(NULL)); // use current time as seed for random generator

  if (!SDL_Init(SDL_INIT_VIDEO))
    return SDL_APP_FAILURE;

  AppState *as = SDL_calloc(1, sizeof(AppState));
  if (!as)
    return SDL_APP_FAILURE;
  *appstate = as;

  as->gameEngine = bootstrapGameEngine(Megabytes(10));
  if (!as->gameEngine->mainArena)
    return SDL_APP_FAILURE;

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH,
                                   SCREEN_HEIGHT, 0, &as->window,
                                   &as->renderer))
    return SDL_APP_FAILURE;

  // FIXME: this should not be done! (it's temporary)
  as->gameEngine->renderer = as->renderer;

  game_engine *gameEngine = as->gameEngine;
  memory_arena *mainArena = gameEngine->mainArena;
  game_context *gameContext = gameEngine->gameContext;

  scene *gameScene = pushStruct(mainArena, scene);
  gameScene->id = 0;
  gameScene->name = pushString(mainArena, "Game Scene");
  gameScene->actionMap = defaultActions;
  gameScene->actionsQueue = pushStruct(mainArena, linked_list_node);
  setCurrentScene(gameContext, gameScene);

  entity_manager *entityManager = getEntityManager(gameContext);
  as->lastStep = SDL_GetTicks();

  // INIT
  gameEngine->addSystem(gameEngine, GAME_ENGINE_INIT, spawnEntities);

  // INPUT
  gameEngine->addSystem(gameEngine, GAME_ENGINE_INPUT, handlePlayerInput);

  // UPDATE
  gameEngine->addSystem(gameEngine, GAME_ENGINE_UPDATE, moveSystem);
  gameEngine->addSystem(gameEngine, GAME_ENGINE_UPDATE, keepInBoundsSystem);

  // RENDER
  gameEngine->addSystem(gameEngine, GAME_ENGINE_RENDER, renderShapeSystem);
  gameEngine->addSystem(gameEngine, GAME_ENGINE_RENDER, renderPlayerSystem);

  // the init is only called once
  gameEngine->init(gameEngine);

  return SDL_APP_CONTINUE;
}

action_kind findAction(scene *currentScene, SDL_Scancode key_code) {
  action_kind found = ACTION_NONE;

  // TODO: use the scene actions
  u16 actionsSize = sizeof(defaultActions) / sizeof(defaultActions[0]);

  for (int i = 0; i < actionsSize; i++) {
    if (defaultActions[i].keycode == key_code) {
      found = defaultActions[i].action;
      break;
    }
  }

  return found;
}

SDL_AppResult handle_key_event(AppState *as, SDL_Event *event) {
  // for now ignore repeated keys
  if (event->key.repeat > 0)
    return SDL_APP_CONTINUE;

  switch (event->key.scancode) {
  case SDL_SCANCODE_Q:
  case SDL_SCANCODE_ESCAPE:
    return SDL_APP_SUCCESS;
  default: {
    scene *currentScene = getCurrentScene(as->gameEngine->gameContext);
    action_kind actionKind = findAction(currentScene, event->key.scancode);
    if (actionKind == ACTION_NONE)
      break;

    entity_manager *em = getEntityManager(as->gameEngine->gameContext);
    action *foundAction = pushStruct(em->gameArena, action);
    foundAction->kind = actionKind;
    foundAction->state = actionStateFromEventType(event->type);
    linked_list_node *action_node = pushStruct(em->gameArena, linked_list_node);
    action_node->value = foundAction;

    pushToLinkedList(em->gameArena, &currentScene->actionsQueue, action_node);

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
  case SDL_EVENT_KEY_UP:
    return handle_key_event(as, event);
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *as = (AppState *)appstate;
  const Uint64 now = SDL_GetTicks();

  // FIXME: all this iterate should be the other way around the game engine
  // should be the main part and SDL just a service for it
  as->gameEngine->preFrame(as->gameEngine);
  // as->gameEngine->input(as->gameEngine);

  while ((now - as->lastStep) >= STEP_RATE_IN_MILLISECONDS) {
    as->gameEngine->update(as->gameEngine);
    as->lastStep += STEP_RATE_IN_MILLISECONDS;
  }

  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 255);
  SDL_RenderClear(as->renderer);

  as->gameEngine->render(as->gameEngine);

  SDL_RenderPresent(as->renderer);

  as->gameEngine->postFrame(as->gameEngine);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate != NULL) {
    AppState *as = (AppState *)appstate;

    as->gameEngine->destroy(as->gameEngine);
    SDL_free(as);
  }
}
