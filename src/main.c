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
#include "ecs/entity.h"
#include "ecs/init.c"
#include "game_engine/game_engine.h"
#include "game_engine/init.c"
#include "memory.h"
#include "types.h"

typedef struct SDL_Offscreen_Buffer {
  SDL_Texture *texture;
  void *memory;
  int width;
  int height;
  int pitch;
} sdl_offscreen_buffer;

typedef struct AppState {
  SDL_Window *window;
  SDL_Renderer *renderer;
  game_engine *gameEngine;
  u64 lastStep;
} AppState;

static sdl_offscreen_buffer GlobalBackBuffer;

// these will be scene dependant
action_map defaultActions[] = {
    {.keycode = SDL_SCANCODE_W, .action = ACTION_UP},
    {.keycode = SDL_SCANCODE_S, .action = ACTION_DOWN},
    {.keycode = SDL_SCANCODE_D, .action = ACTION_RIGHT},
    {.keycode = SDL_SCANCODE_A, .action = ACTION_LEFT},
};

void fullLoop(AppState *as);

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

void ResizeTexture(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer,
                   int Width, int Height) {

  if (Buffer->texture) {
    SDL_DestroyTexture(Buffer->texture);
  }
  if (Buffer->memory) {
    SDL_free(Buffer->memory);
  }

  int BytesPerPixel = 4;
  Buffer->width = Width;
  Buffer->height = Height;

  Buffer->texture =
      SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, Width, Height);
  // TODO: handle the fail case

  int BitmapMemorySize = (Buffer->width * Buffer->height) * BytesPerPixel;
  Buffer->memory = SDL_malloc(BitmapMemorySize);
  Buffer->pitch = Width * BytesPerPixel;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  srand(time(NULL)); // use current time as seed for random generator

  if (!SDL_Init(SDL_INIT_VIDEO))
    return SDL_APP_FAILURE;

  AppState *as = SDL_calloc(1, sizeof(AppState));
  if (!as)
    return SDL_APP_FAILURE;
  *appstate = as;

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE,
                                   &as->window, &as->renderer))
    return SDL_APP_FAILURE;

  ResizeTexture(&GlobalBackBuffer, as->renderer, 1280, 720);

  as->gameEngine = bootstrapGameEngine(Megabytes(10));
  if (!as->gameEngine)
    return SDL_APP_FAILURE;

  as->gameEngine->backBuffer->width = GlobalBackBuffer.width;
  as->gameEngine->backBuffer->height = GlobalBackBuffer.height;
  as->gameEngine->backBuffer->memory = GlobalBackBuffer.memory;
  as->gameEngine->backBuffer->pitch = GlobalBackBuffer.pitch;

  as->lastStep = SDL_GetTicks();
  GameEngine_init(as->gameEngine);

  return SDL_APP_CONTINUE;
}

action_kind findAction(SDL_Scancode key_code) {
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
  /*case SDL_SCANCODE_A:*/
  /*  fullLoop(as);*/
  /*  return SDL_APP_CONTINUE;*/
  default: {
    action_kind actionKind = findAction(event->key.scancode);
    if (actionKind == ACTION_NONE)
      break;

    entity_manager *em = as->gameEngine->entityManager;
    action *foundAction = pushStruct(em->gameArena, action);
    foundAction->kind = actionKind;
    foundAction->state = actionStateFromEventType(event->type);
    linked_list_node *action_node = pushStruct(em->gameArena, linked_list_node);
    action_node->value = foundAction;
    // TODO: add an entity with the found action and link it to the current
    // player somehow

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
static void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer,
                            sdl_offscreen_buffer *Buffer) {
  SDL_UpdateTexture(Buffer->texture, 0, Buffer->memory, Buffer->pitch);

  SDL_RenderTexture(Renderer, Buffer->texture, 0, 0);

  SDL_RenderPresent(Renderer);
}

void fullLoop(AppState *as) {
  const Uint64 now = SDL_GetTicks();

  // NOTE: is this the correct way to handle timing?
  /*while ((now - as->lastStep) >= STEP_RATE_IN_MILLISECONDS) {*/
  /*  GameEngine_update(as->gameEngine);*/
  /*  as->lastStep += STEP_RATE_IN_MILLISECONDS;*/
  /*}*/

  GameEngine_render(as->gameEngine);

  SDLUpdateWindow(as->window, as->renderer, &GlobalBackBuffer);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  fullLoop((AppState *)appstate);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate != NULL) {
    AppState *as = (AppState *)appstate;

    GameEngine_destroy(as->gameEngine);
    SDL_free(as);
  }
}
