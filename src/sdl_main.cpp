#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "base.cpp"

#define Assert(Expression) SDL_assert(Expression)

#include "base_types.h"
#include "constants.h"
#include "ecs/init.cpp"
#include "game_engine/game_engine.h"
#include "game_engine/init.cpp"
#include "game_engine/types.h"
#include "memory.cpp"
#include "memory.h"
#include "mymath.cpp"

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
  // even if the 0 is always the keyboard, for convenience we create an array of
  // the same size for the JoysticIds
  SDL_JoystickID JoysticIdsByIndex[MAX_N_CONTROLLERS];
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

internal void MySDL_ResizeTexture(sdl_offscreen_buffer *Buffer,
                                  SDL_Renderer *Renderer, int Width,
                                  int Height) {
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

  MySDL_ResizeTexture(&GlobalBackBuffer, as->renderer, 1280, 720);
  as->gameEngine->BackBuffer->width = GlobalBackBuffer.width;
  as->gameEngine->BackBuffer->height = GlobalBackBuffer.height;
  as->gameEngine->BackBuffer->memory = GlobalBackBuffer.memory;
  as->gameEngine->BackBuffer->pitch = GlobalBackBuffer.pitch;

  Wayne_init(as->gameEngine, SDL_GetTicks());

  // init the keyboard controller that for now is always active
  GlobalControllers.Controllers[0] =
      wayne_controller_input{.IsActive = true, .IsAnalog = false};
  GlobalControllers.ControllersCount++;

  return SDL_APP_CONTINUE;
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
    break;
  }
  }
  return SDL_APP_CONTINUE;
}

void MySDL_AddJoystick(SDL_Event *event, sdl_controllers_mapping *Controllers) {
  const SDL_JoystickID JoystickId = event->jdevice.which;
  SDL_Log("Joystick added %d", JoystickId);
  SDL_Joystick *Joystick = SDL_OpenJoystick(JoystickId);
  if (!Joystick) {
    SDL_Log("Joystick added but not opened %d error %s", JoystickId,
            SDL_GetError());
  }

  int ControllerIndex = Controllers->ControllersCount;
  // NOTE: I know it can be done all in the previous line but for now I prefer
  // to be explicit about it
  Controllers->ControllersCount++;

  Controllers->JoysticIdsByIndex[ControllerIndex] = JoystickId;
  wayne_controller_input *CurrentController =
      &Controllers->Controllers[ControllerIndex];

  CurrentController->IsActive = true;
}

wayne_controller_input *
ControllerByJoystickId(SDL_JoystickID JoystickId,
                       sdl_controllers_mapping *Controllers) {
  for (int ControllerIndex = 0; ControllerIndex < Controllers->ControllersCount;
       ControllerIndex++) {
    if (Controllers->JoysticIdsByIndex[ControllerIndex] == JoystickId)
      return &Controllers->Controllers[ControllerIndex];
  }
  return NULL;
}

float MySDL_NormalizeJoystickAxis(int16_t value) {
  int8 Direction = value > 0 ? 1 : -1;
  float DeadZoneThreshold = 0.25 * Direction;
  float Factor = value > 0 ? SDL_JOYSTICK_AXIS_MAX : SDL_JOYSTICK_AXIS_MIN;

  float NormalizedValue = ((float)value / Factor) * Direction;

  if (NormalizedValue < 0 && NormalizedValue > DeadZoneThreshold)
    return 0.0;

  if (NormalizedValue > 0 && NormalizedValue < DeadZoneThreshold)
    return 0.0;

  return NormalizedValue;
}

void MySDL_HandleButtonEvent(SDL_Event *event,
                             sdl_controllers_mapping *Controllers) {
  // TODO: handle up and down with the transitions counter

  wayne_controller_input *CurrentController =
      ControllerByJoystickId(event->jaxis.which, Controllers);
  if (!CurrentController)
    return;

  switch (event->jbutton.button) {
  case 0: {
    CurrentController->ButtonSouth.isDown = event->jbutton.down;
  } break;

  case 1: {
    CurrentController->ButtonEast.isDown = event->jbutton.down;
  } break;

  case 2: {
    CurrentController->ButtonWest.isDown = event->jbutton.down;
  } break;

  case 3: {
    CurrentController->ButtonNorth.isDown = event->jbutton.down;
  } break;
  }
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState *as = (AppState *)appstate;
  switch (event->type) {

  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;

  case SDL_EVENT_JOYSTICK_ADDED: {
    MySDL_AddJoystick(event, &GlobalControllers);
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
    wayne_controller_input *CurrentController =
        ControllerByJoystickId(event->jaxis.which, &GlobalControllers);
    SDL_JoyAxisEvent AxisEvent = event->jaxis;
    if (CurrentController) {
      CurrentController->IsAnalog = true;

      if (AxisEvent.axis == 0)
        CurrentController->StickY =
            MySDL_NormalizeJoystickAxis(AxisEvent.value);

      if (AxisEvent.axis == 1)
        CurrentController->StickX =
            MySDL_NormalizeJoystickAxis(AxisEvent.value);
    }
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_BUTTON_UP:
  case SDL_EVENT_JOYSTICK_BUTTON_DOWN: {
    MySDL_HandleButtonEvent(event, &GlobalControllers);
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
