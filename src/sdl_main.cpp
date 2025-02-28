#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "base.cpp"
#include "base_types.h"
#include "constants.h"
#include "ecs/entity.h"
#include "ecs/init.cpp"
#include "game_engine/game_engine.h"
#include "game_engine/init.cpp"
#include "game_engine/types.h"
#include "memory.cpp"
#include "memory.h"
#include "mymath.cpp"
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
  wayne_t *gameEngine;
} AppState;

struct sdl_controllers_mapping {
  int ControllersCount;
  wayne_controller_input Controllers[MAX_N_CONTROLLERS];
};

#define MyInitFlags SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK

// TODO: do not use globals if not necessary, remove the one that arent;
// these are just for convenience and fast prototype
global_variable sdl_offscreen_buffer GlobalBackBuffer;
global_variable SDL_AudioDeviceID GlobalAudioDeviceID;
global_variable SDL_AudioStream *GlobalAudioStream;
global_variable wayne_audio_buffer GlobalAudioBuffer;

global_variable sdl_controllers_mapping GlobalControllers;

// these will be scene dependant
action_map defaultActions[] = {
    {.keycode = SDL_SCANCODE_W, .action = ACTION_UP},
    {.keycode = SDL_SCANCODE_S, .action = ACTION_DOWN},
    {.keycode = SDL_SCANCODE_D, .action = ACTION_RIGHT},
    {.keycode = SDL_SCANCODE_A, .action = ACTION_LEFT},
};

action_state actionStateFromEventType(u32 eventType) {
  switch (eventType) {
  case SDL_EVENT_KEY_DOWN:
    return ACTION_STATE_START;
  case SDL_EVENT_KEY_UP:
    return ACTION_STATE_STOP;
  default:
    return ACTION_STATE_NONE;
  }
}

internal void SDLResizeTexture(sdl_offscreen_buffer *Buffer,
                               SDL_Renderer *Renderer, int Width, int Height) {
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

internal int MySDL_InitSoundBuffer(wayne_audio_buffer *Buffer) {
  // temporary solution, it should be done outside of here
  Buffer->SamplesPerSecond = 48000;
  Buffer->ToneHz = 260;
  Buffer->ToneVolume = 100;
  Buffer->Channels = 2;
  Buffer->BytesPerSample = sizeof(float) * Buffer->Channels;
  Buffer->BufferSize = Buffer->BytesPerSample * Buffer->SamplesPerSecond;
  Buffer->WavePeriod = Buffer->SamplesPerSecond / Buffer->ToneHz;

  // open an audio device
  SDL_AudioSpec spec = {
      .format = SDL_AUDIO_F32,
      .channels = Buffer->Channels,
      .freq = Buffer->SamplesPerSecond,
  };

  Buffer->Data = SDL_malloc(Buffer->BufferSize);
  if (!Buffer->Data) {
    SDL_Log("Cannot initialize audio buffer %s", SDL_GetError());
    return 0;
  }

  // create an audio stream
  GlobalAudioStream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
  if (!GlobalAudioStream) {
    SDL_Log("Failed to create audio stream: %s", SDL_GetError());
    return 0;
  }

  SDL_ClearAudioStream(GlobalAudioStream);
  SDL_SetAudioStreamGain(GlobalAudioStream, 0.1f);
  SDL_ResumeAudioStreamDevice(GlobalAudioStream);

  return 1;
}

internal void MySDL_FillSoundBuffer(wayne_audio_buffer *Buffer) {
  // There are crackling sounds at the start and at the end even the function
  // seems to ouput the correct data, for now remove the audio and think about
  // it later
  // return;

  SDL_PutAudioStreamData(GlobalAudioStream, Buffer->Data, Buffer->BufferSize);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  srand(time(NULL)); // use current time as seed for random generator

  if (!SDL_Init(MyInitFlags))
    return SDL_APP_FAILURE;

  AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
  if (!as)
    return SDL_APP_FAILURE;
  *appstate = as;

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE,
                                   &as->window, &as->renderer))
    return SDL_APP_FAILURE;

  if (!MySDL_InitSoundBuffer(&GlobalAudioBuffer))
    return SDL_APP_FAILURE;

  as->gameEngine = bootstrapWayne(Megabytes(10));
  if (!as->gameEngine)
    return SDL_APP_FAILURE;

  as->gameEngine->AudioBuffer = &GlobalAudioBuffer;

  SDLResizeTexture(&GlobalBackBuffer, as->renderer, 1280, 720);
  as->gameEngine->BackBuffer->width = GlobalBackBuffer.width;
  as->gameEngine->BackBuffer->height = GlobalBackBuffer.height;
  as->gameEngine->BackBuffer->memory = GlobalBackBuffer.memory;
  as->gameEngine->BackBuffer->pitch = GlobalBackBuffer.pitch;

  Wayne_init(as->gameEngine, SDL_GetTicks());

  return SDL_APP_CONTINUE;
}

action_kind findAction(SDL_Scancode key_code) {
  action_kind found = ACTION_NONE;

  // TODO: use the scene actions
  u16 actionsSize = sizeof(defaultActions) / sizeof(defaultActions[0]);

  for (int i = 0; i < actionsSize; i++) {
    if (defaultActions[i].keycode == key_code) {
      found = (action_kind)defaultActions[i].action;
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
  AppState *as = (AppState *)appstate;
  switch (event->type) {

  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;

  case SDL_EVENT_JOYSTICK_ADDED: {
    const SDL_JoystickID Id = event->jdevice.which;
    SDL_Log("Joystick added %d", Id);
    SDL_Joystick *joystick = SDL_OpenJoystick(Id);
    if (!joystick) {
      SDL_Log("Joystick added but not opened %d error %s", Id, SDL_GetError());
    }
    // FIXME: handle the mapping of multiple controllers id/index, the id given
    // by sdl could change if the controller is disconnected/reconnected, for
    // now I'll use just the controller 0 and don't care about the index, I just
    // want to see stuff moving
    wayne_controller_input *CurrentController =
        &GlobalControllers.Controllers[0];

    CurrentController->isActive = true;
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
    wayne_controller_input *CurrentController =
        &GlobalControllers.Controllers[0];

    CurrentController->isAnalog = true;
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_BUTTON_UP:
  case SDL_EVENT_JOYSTICK_BUTTON_DOWN: {
    wayne_controller_input *CurrentController =
        &GlobalControllers.Controllers[0];

    if (event->jbutton.button == 0)
      CurrentController->ButtonSouth.isDown = event->jbutton.down;
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_REMOVED: {
    // TODO: handle me
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    return handle_key_event(as, event);
  }
  return SDL_APP_CONTINUE;
}

internal void MySDL_UpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer,
                                 sdl_offscreen_buffer *Buffer) {
  SDL_UpdateTexture(Buffer->texture, 0, Buffer->memory, Buffer->pitch);

  SDL_RenderTexture(Renderer, Buffer->texture, 0, 0);

  SDL_RenderPresent(Renderer);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *as = (AppState *)appstate;

  const Uint64 now = SDL_GetTicks();

  // FIXME: from the code is not clear that the audio buffer is being updated
  // most probably I'm doing something wrong because it should be clear! fix it
  // later
  Wayne_updateAndRender(as->gameEngine, now, GlobalControllers.Controllers);

  // we have a pointer to the GlobalAudioBuffer inside the gameEngine and
  // we update that with the Wayne_updateAndRender function
  MySDL_FillSoundBuffer(&GlobalAudioBuffer);
  MySDL_UpdateWindow(as->window, as->renderer, &GlobalBackBuffer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate != NULL) {
    AppState *as = (AppState *)appstate;

    Wayne_destroy(as->gameEngine);
    SDL_free(as);
  }
}
