#include <SDL3/SDL_loadso.h>
#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "shapes_destroyer/base.h"

#define Assert(Expression) SDL_assert(Expression)

#include "shapes_destroyer/constants.h"
#include "shapes_destroyer/game_engine/game_engine.h"
#include "shapes_destroyer/types.h"

typedef struct sdl_offscreen_buffer {
  SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
} sdl_offscreen_buffer;

struct sdl_controllers_mapping {
  int ControllersCount;
  // even if the 0 is always the keyboard, for convenience we create an array of
  // the same size for the JoysticIds
  SDL_JoystickID JoysticIdsByIndex[MAX_N_CONTROLLERS];
  wayne_controller_input Controllers[MAX_N_CONTROLLERS];
};

typedef struct app_state {
  SDL_Window *Window;
  SDL_Renderer *Renderer;
  wayne_t *GameEngine;
  SDL_AudioStream *AudioStream;
  memory_arena *GamePermanentStorage;
  memory_arena *GameTransientStorage;

  sdl_offscreen_buffer BackBuffer;
  SDL_AudioDeviceID AudioDeviceId;
  wayne_audio_buffer AudioBuffer;
  sdl_controllers_mapping Controllers;

  bool AudioIsPaused;
} app_state;

struct game_code {
  SDL_SharedObject *CodeHandle;
  SDL_Time LastModified;

  char *FullLibPath;

  wayne_bootstrap *Bootstrap;
  wayne_init *Init;
  wayne_update_and_render *UpdateAndRender;
  wayne_destroy *Destroy;

  bool IsValid;
};

global_variable game_code GameCode;

#define MyInitFlags SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK

// TODO: do not use globals if not necessary, remove the one that arent;
// these are just for convenience and fast prototype

internal void MySDL_ResizeTexture(sdl_offscreen_buffer *Buffer,
                                  SDL_Renderer *Renderer, int Width,
                                  int Height) {
  if (Buffer->Texture) {
    SDL_DestroyTexture(Buffer->Texture);
  }
  if (Buffer->Memory) {
    SDL_free(Buffer->Memory);
  }

  int BytesPerPixel = 4;
  Buffer->Width = Width;
  Buffer->Height = Height;

  Buffer->Texture =
      SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, Width, Height);
  // TODO: handle the fail case
  Assert(Buffer->Texture);

  int NumberOfPixels = Buffer->Width * Buffer->Height;
  Buffer->Memory = SDL_calloc(NumberOfPixels, BytesPerPixel);
  SDL_memset4(Buffer->Memory, 0, NumberOfPixels * BytesPerPixel);

  Buffer->Pitch = Width * BytesPerPixel;
}

internal int MySDL_InitSoundBuffer(wayne_audio_buffer *Buffer,
                                   SDL_AudioStream **OutAudioStream) {
  // open an audio device
  SDL_AudioSpec Spec = {
      .format = SDL_AUDIO_F32,
      .channels = Buffer->Channels,
      .freq = Buffer->SamplesPerSecond,
  };

  // create an audio stream
  *OutAudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                              &Spec, NULL, Buffer->Data);
  if (!*OutAudioStream) {
    SDL_Log("Failed to create audio stream: %s", SDL_GetError());
    return 0;
  }

  // SDL_ClearAudioStream(*OutAudioStream);
  SDL_SetAudioStreamGain(*OutAudioStream, 0.1f);

  return 1;
}

internal void MySDL_FillSoundBuffer(SDL_AudioStream *AudioStream,
                                    wayne_audio_buffer *Buffer) {
  // FIXME: the audio doesn't work properly at the start and quit of the app
  return;
  SDL_PutAudioStreamData(AudioStream, Buffer->Data, Buffer->BufferSize);
}

internal void CatStrings(const char *StringA, const char *StringB, char *Dest) {
  for (const char *a = StringA; *a; a++) {
    *Dest++ = *a;
  }

  for (const char *b = StringB; *b; b++) {
    *Dest++ = *b;
  }

  // null terminator
  *Dest = 0;
}

internal void MySDL_LoadGameCode(game_code *LocalGameCode) {
  const char *LibPath = (const char *)LocalGameCode->FullLibPath;
  SDL_PathInfo LibInfo = {};
  if (!SDL_GetPathInfo(LibPath, &LibInfo)) {
    SDL_Log("Cannot get info on the file %s - error: %s", LibPath,
            SDL_GetError());
    return;
  }

  if (LocalGameCode->LastModified == LibInfo.modify_time) {
    // nothing has changed
    return;
  }

  if (LocalGameCode->CodeHandle) {
    SDL_UnloadObject(LocalGameCode->CodeHandle);
    LocalGameCode->IsValid = false;
  }

  LocalGameCode->CodeHandle =
      SDL_LoadObject((const char *)LocalGameCode->FullLibPath);
  if (!LocalGameCode->CodeHandle) {
    SDL_Log("Cannot load the game code %s", SDL_GetError());
  }
  LocalGameCode->LastModified = LibInfo.modify_time;

  LocalGameCode->Bootstrap = (wayne_bootstrap *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_bootstrap");
  LocalGameCode->Init =
      (wayne_init *)SDL_LoadFunction(LocalGameCode->CodeHandle, "Wayne_init");
  LocalGameCode->UpdateAndRender = (wayne_update_and_render *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_updateAndRender");
  LocalGameCode->Destroy = (wayne_destroy *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_destroy");

  if (LocalGameCode->Bootstrap && LocalGameCode->Init &&
      LocalGameCode->UpdateAndRender && LocalGameCode->Destroy) {
    LocalGameCode->IsValid = true;
  }

  if (!LocalGameCode->IsValid) {
    LocalGameCode->Bootstrap = Wayne_bootsrapStub;
    LocalGameCode->Init = Wayne_initStub;
    LocalGameCode->UpdateAndRender = Wayne_updateAndRenderStub;
    LocalGameCode->Destroy = Wayne_destroyStub;

    SDL_Log("Cannot load the game code %s", SDL_GetError());
    // we don't need to return anything because the code should work with the
    // stub and in the future will hot reload the code anyway
  }
}

internal memory_arena *MySDL_CreateMemoryArena(memory_size TotalSize) {
  memory_arena *Arena = (memory_arena *)SDL_calloc(1, sizeof(memory_arena));
  if (!Arena) {
    // TODO: log error
    return NULL;
  }

  Arena->startAddress = (u8 *)SDL_malloc(TotalSize);
  if (!Arena->startAddress) {
    // TODO: log error
    return NULL;
  }

  Arena->totalSize = TotalSize;
  Arena->used = 0;
  SDL_memset(Arena->startAddress, 0, TotalSize);
  return Arena;
}

SDL_AppResult SDL_AppInit(void **AppState, int Argc, char **Argv) {
  srand(time(NULL));  // use current time as seed for random generator

  if (!SDL_Init(MyInitFlags)) return SDL_APP_FAILURE;

  app_state *As = (app_state *)SDL_calloc(1, sizeof(app_state));
  if (!As) return SDL_APP_FAILURE;
  *AppState = As;

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE,
                                   &As->Window, &As->Renderer))
    return SDL_APP_FAILURE;

  GameCode = {0};
  const char *CurrentBasePath = SDL_GetBasePath();
  size_t CurrentPathSize = strlen(CurrentBasePath);

  // TODO: fix the output of the game code as library
  const char *RelativeLibPath =
      "src/shapes_destroyer/libshapes_destroyer.dylib";
  size_t RelativeLibPathSize = strlen(RelativeLibPath);

  // FIXME: this seems so wrong
  GameCode.FullLibPath =
      (char *)SDL_calloc(RelativeLibPathSize + CurrentPathSize, 1);

  CatStrings(CurrentBasePath, RelativeLibPath, GameCode.FullLibPath);
  MySDL_LoadGameCode(&GameCode);

  {
    // TODO: extract me somewhere
    wayne_audio_buffer *Buffer = &As->AudioBuffer;
    // temporary solution, it should be done outside of here
    Buffer->SamplesPerSecond = 48000;
    Buffer->ToneHz = 260;
    Buffer->ToneVolume = 100;
    Buffer->Channels = 2;
    Buffer->BytesPerSample = sizeof(float) * Buffer->Channels;
    Buffer->BufferSize = Buffer->BytesPerSample * Buffer->SamplesPerSecond;
    Buffer->WavePeriod = Buffer->SamplesPerSecond / Buffer->ToneHz;

    Buffer->Data = SDL_calloc(1, Buffer->BufferSize);
  }
  if (!As->AudioBuffer.Data) {
    SDL_Log("Cannot initialize audio buffer %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!MySDL_InitSoundBuffer(&As->AudioBuffer, &As->AudioStream))
    return SDL_APP_FAILURE;

  MySDL_ResizeTexture(&As->BackBuffer, As->Renderer, 1280, 720);

  As->GamePermanentStorage = MySDL_CreateMemoryArena(Megabytes(64));
  As->GameTransientStorage = MySDL_CreateMemoryArena(Gigabytes(4));

  As->GameEngine =
      GameCode.Bootstrap(As->GamePermanentStorage, As->GameTransientStorage);

  if (As->GameEngine) {
    As->GameEngine->AudioBuffer = &As->AudioBuffer;

    As->GameEngine->BackBuffer->Width = As->BackBuffer.Width;
    As->GameEngine->BackBuffer->Height = As->BackBuffer.Height;
    As->GameEngine->BackBuffer->Memory = As->BackBuffer.Memory;
    As->GameEngine->BackBuffer->Pitch = As->BackBuffer.Pitch;
  }

  GameCode.Init(As->GameEngine, SDL_GetTicks());

  MySDL_FillSoundBuffer(As->AudioStream, &As->AudioBuffer);
  // resume the audio stream once is filled
  SDL_ResumeAudioStreamDevice(As->AudioStream);
  As->AudioIsPaused = false;

  // init the keyboard controller that for now is always active
  wayne_controller_input DefaultController = {0};
  DefaultController.IsActive = true;
  DefaultController.IsAnalog = false;
  As->Controllers.Controllers[0] = DefaultController;
  As->Controllers.ControllersCount++;

  return SDL_APP_CONTINUE;
}

void SetButtonState(SDL_Event *Event, wayne_controller_button *Button) {
  Button->HalfTransitionCount = Event->key.repeat;
  Button->isDown = Event->key.down;
}

SDL_AppResult MySDL_HandleKeyEvent(SDL_Event *Event,
                                   sdl_controllers_mapping *Controllers,
                                   app_state *AppState) {
  wayne_controller_input *Keyboard = &Controllers->Controllers[0];

  switch (Event->key.scancode) {
    case SDL_SCANCODE_Q:
    case SDL_SCANCODE_ESCAPE:
      return SDL_APP_SUCCESS;

    case SDL_SCANCODE_W: {
      SetButtonState(Event, &Keyboard->MoveUp);
    } break;

    case SDL_SCANCODE_S: {
      SetButtonState(Event, &Keyboard->MoveDown);
    } break;

    case SDL_SCANCODE_A: {
      SetButtonState(Event, &Keyboard->MoveLeft);
    } break;

    case SDL_SCANCODE_D: {
      SetButtonState(Event, &Keyboard->MoveRight);
    } break;

    case SDL_SCANCODE_P: {
      // ignore repeated keys
      if (Event->key.repeat > 0) break;

      if (!Event->key.down) break;

      if (AppState->AudioIsPaused) {
        SDL_ResumeAudioStreamDevice(AppState->AudioStream);
      } else {
        SDL_PauseAudioStreamDevice(AppState->AudioStream);
      }
      AppState->AudioIsPaused = !AppState->AudioIsPaused;
    } break;

    default: {
      break;
    }
  }
  return SDL_APP_CONTINUE;
}

void MySDL_AddJoystick(SDL_Event *Event, sdl_controllers_mapping *Controllers) {
  const SDL_JoystickID JoystickId = Event->jdevice.which;
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

wayne_controller_input *ControllerByJoystickId(
    SDL_JoystickID JoystickId, sdl_controllers_mapping *Controllers) {
  for (int ControllerIndex = 0; ControllerIndex < Controllers->ControllersCount;
       ControllerIndex++) {
    if (Controllers->JoysticIdsByIndex[ControllerIndex] == JoystickId)
      return &Controllers->Controllers[ControllerIndex];
  }
  return NULL;
}

float MySDL_NormalizeJoystickAxis(int16_t Value) {
  int8 Direction = Value > 0 ? 1 : -1;
  float DeadZoneThreshold = 0.25 * Direction;
  float Factor = Value > 0 ? SDL_JOYSTICK_AXIS_MAX : SDL_JOYSTICK_AXIS_MIN;

  float NormalizedValue = ((float)Value / Factor) * Direction;

  if (NormalizedValue < 0 && NormalizedValue > DeadZoneThreshold) return 0.0;

  if (NormalizedValue > 0 && NormalizedValue < DeadZoneThreshold) return 0.0;

  return NormalizedValue;
}

void MySDL_HandleButtonEvent(SDL_Event *Event,
                             sdl_controllers_mapping *Controllers) {
  // TODO: handle up and down with the transitions counter

  wayne_controller_input *CurrentController =
      ControllerByJoystickId(Event->jaxis.which, Controllers);
  if (!CurrentController) return;

  switch (Event->jbutton.button) {
    case 0: {
      CurrentController->ButtonSouth.HalfTransitionCount++;
      CurrentController->ButtonSouth.isDown = Event->jbutton.down;
    } break;

    case 1: {
      CurrentController->ButtonSouth.HalfTransitionCount++;
      CurrentController->ButtonEast.isDown = Event->jbutton.down;
    } break;

    case 2: {
      CurrentController->ButtonSouth.HalfTransitionCount++;
      CurrentController->ButtonWest.isDown = Event->jbutton.down;
    } break;

    case 3: {
      CurrentController->ButtonSouth.HalfTransitionCount++;
      CurrentController->ButtonNorth.isDown = Event->jbutton.down;
    } break;
  }
}

SDL_AppResult SDL_AppEvent(void *AppState, SDL_Event *Event) {
  app_state *As = (app_state *)AppState;
  switch (Event->type) {
    case SDL_EVENT_QUIT:
      return SDL_APP_SUCCESS;

    case SDL_EVENT_JOYSTICK_ADDED: {
      MySDL_AddJoystick(Event, &As->Controllers);
    }
      return SDL_APP_CONTINUE;

    case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
      wayne_controller_input *CurrentController =
          ControllerByJoystickId(Event->jaxis.which, &As->Controllers);
      SDL_JoyAxisEvent AxisEvent = Event->jaxis;
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
      MySDL_HandleButtonEvent(Event, &As->Controllers);
    }
      return SDL_APP_CONTINUE;

    case SDL_EVENT_JOYSTICK_REMOVED: {
      // TODO: handle me
    }
      return SDL_APP_CONTINUE;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
      return MySDL_HandleKeyEvent(Event, &As->Controllers, As);
  }
  return SDL_APP_CONTINUE;
}

internal void MySDL_UpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer,
                                 sdl_offscreen_buffer *Buffer) {
  SDL_UpdateTexture(Buffer->Texture, 0, Buffer->Memory, Buffer->Pitch);

  SDL_RenderTexture(Renderer, Buffer->Texture, 0, 0);

  SDL_RenderPresent(Renderer);
}

SDL_AppResult SDL_AppIterate(void *AppState) {
  app_state *As = (app_state *)AppState;
  const Uint64 Now = SDL_GetTicks();

  // reset the transient storage
  As->GameTransientStorage->used = 0;

  GameCode.UpdateAndRender(As->GameEngine, Now, As->Controllers.Controllers);

  MySDL_FillSoundBuffer(As->AudioStream, &As->AudioBuffer);
  MySDL_UpdateWindow(As->Window, As->Renderer, &As->BackBuffer);

  for (int i = 0; i < As->Controllers.ControllersCount; i++) {
    // reset the half transitions count
    wayne_controller_input *Controller = &As->Controllers.Controllers[i];
    for (int ButtonIndex = 0; ButtonIndex < ArraySize(Controller->Buttons);
         ButtonIndex++) {
      Controller->Buttons[ButtonIndex].HalfTransitionCount = 0;
    }
  }

  // The hot reload will not work with how the systems are implemented now,
  // because we're calling them through pointers and not directly so we need to
  // find a solution for that. BUT BEFORE GOING INTO THE RABBIT HOLE try to call
  // the systems directly
  //
  // TODO: record this victory in a victory log??
  // 2025-03-03: Hippye!!! is how I tought passin the system callback by
  // pointer will prevent the hot reload to working properly
  //
  // there is another problem with the hot reload, we're storing the offsets as
  // local_persists and they refresh at every frame (for now) fix that too
  MySDL_LoadGameCode(&GameCode);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *AppState, SDL_AppResult Result) {
  if (AppState != NULL) {
    app_state *As = (app_state *)AppState;
    SDL_PauseAudioStreamDevice(As->AudioStream);
    GameCode.Destroy(As->GameEngine);
    SDL_free(As->AudioBuffer.Data);
    SDL_free(As->GameTransientStorage);
    SDL_free(As->GamePermanentStorage);
    SDL_free(As);
  }
}
