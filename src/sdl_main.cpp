#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "base.h"
#include "constants.h"
#include "types.h"

#if HANDMADE_HERO
#include "my_handmade_hero/my_handmade_hero.h"
typedef void game_engine;
#else
#include "shapes_destroyer/game_engine/game_engine.h"
typedef wayne_controller_input game_controller_input;
typedef wayne_controller_button game_controller_button;
typedef wayne_t game_engine;
#endif

#define Assert(Expression) SDL_assert(Expression)

#define GLOBAL_AUDIO_GAIN 0.1f

typedef struct sdl_offscreen_buffer {
  SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  u8 BytesPerPixel;
} sdl_offscreen_buffer;

struct sdl_audio_buffer {
  SDL_AudioStream *AudioStream;
  void *Data;

  SDL_AudioDeviceID AudioDeviceId;
  u8 Channels;
  u32 BufferSize;
  int BytesPerSample;
  int SamplesPerSecond;
  int ToneHz;
  int ToneVolume;
  int WavePeriod;
};

struct sdl_controllers_mapping {
  int ControllersCount;
  // even if the 0 is always the keyboard, for convenience we create an array of
  // the same size for the JoysticIds
  SDL_JoystickID JoysticIdsByIndex[MAX_N_CONTROLLERS];
  game_controller_input Controllers[MAX_N_CONTROLLERS];
};

typedef struct app_state {
  SDL_Window *Window;
  SDL_Renderer *Renderer;
  game_engine *GameEngine;
  memory_arena *GamePermanentStorage;
  memory_arena *GameTransientStorage;

  sdl_controllers_mapping *NewInput;
  sdl_controllers_mapping *OldInput;
  sdl_controllers_mapping Controllers[2];

  f32 TargetRefreshRate;
  f32 TargetMsPerFrame;

  sdl_offscreen_buffer BackBuffer;
  sdl_audio_buffer AudioBuffer;

  char *RecordFilename;
  SDL_IOStream *RecordStream;
  size_t RecordOffsetFromStart;

  char *PlaybackFilename;
  SDL_IOStream *PlaybackStream;
  size_t PlaybackOffsetFromStart;

  u64 LastTick;

  bool AudioIsPaused;
  bool InputIsRecording;
  bool InputIsPlaying;
} app_state;

#if HANDMADE_HERO
struct game_code {
  SDL_SharedObject *CodeHandle;
  SDL_Time LastModified;

  char *FullLibPath;

  game_update_and_render *UpdateAndRender;

  bool IsValid;
  bool JustReloaded;
};

global_variable game_code GameCode;
#else
struct game_code {
  SDL_SharedObject *CodeHandle;
  SDL_Time LastModified;

  char *FullLibPath;

  wayne_bootstrap *Bootstrap;
  wayne_update_and_render *UpdateAndRender;
  wayne_reset_systems *ResetSystems;
  wayne_destroy *Destroy;

  bool IsValid;
  bool JustReloaded;
};

global_variable game_code GameCode;
#endif

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

  Buffer->BytesPerPixel = 4;
  Buffer->Width = Width;
  Buffer->Height = Height;

  Buffer->Texture =
      SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, Width, Height);
  // TODO: handle the fail case
  Assert(Buffer->Texture);

  int NumberOfPixels = Buffer->Width * Buffer->Height;
  Buffer->Memory = SDL_calloc(NumberOfPixels, Buffer->BytesPerPixel);
  SDL_memset4(Buffer->Memory, 0, NumberOfPixels * Buffer->BytesPerPixel);

  Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal int MySDL_InitSoundBuffer(sdl_audio_buffer *Buffer) {
  // open an audio device
  SDL_AudioSpec Spec = {
      .format = SDL_AUDIO_F32,
      .channels = Buffer->Channels,
      .freq = Buffer->SamplesPerSecond,
  };

  // create an audio stream
  Buffer->AudioStream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &Spec, NULL, Buffer->Data);
  if (!Buffer->AudioStream) {
    SDL_Log("Failed to create audio stream: %s", SDL_GetError());
    return 0;
  }

  SDL_ClearAudioStream(Buffer->AudioStream);
  SDL_SetAudioStreamGain(Buffer->AudioStream, GLOBAL_AUDIO_GAIN);

  return 1;
}

internal void MySDL_FillSoundBuffer(sdl_audio_buffer *AudioBuffer) {
  // FIXME: the audio doesn't work properly at the start and quit of the app
  return;
  SDL_PutAudioStreamData(AudioBuffer->AudioStream, AudioBuffer->Data,
                         AudioBuffer->BufferSize);
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

#if HANDMADE_HERO
  LocalGameCode->UpdateAndRender = (game_update_and_render *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "UpdateAndRender");
#else
  LocalGameCode->Bootstrap = (wayne_bootstrap *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_bootstrap");
  LocalGameCode->UpdateAndRender = (wayne_update_and_render *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_updateAndRender");
  LocalGameCode->Destroy = (wayne_destroy *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_destroy");
  LocalGameCode->ResetSystems = (wayne_reset_systems *)SDL_LoadFunction(
      LocalGameCode->CodeHandle, "Wayne_ResetSystems");
#endif

#if HANDMADE_HERO
  bool LoadedEverything = LocalGameCode->UpdateAndRender;
#else
  bool LoadedEverything = LocalGameCode->Bootstrap &&
                          LocalGameCode->UpdateAndRender &&
                          LocalGameCode->Destroy && LocalGameCode->ResetSystems;
#endif

  if (LoadedEverything) {
    LocalGameCode->IsValid = true;
    LocalGameCode->JustReloaded = true;
  }

  if (!LocalGameCode->IsValid) {
    LocalGameCode->UpdateAndRender = 0;
#if !HANDMADE_HERO
    LocalGameCode->Bootstrap = 0;
    LocalGameCode->Destroy = 0;
    LocalGameCode->ResetSystems = 0;
#endif

    SDL_Log("Cannot load the game code %s", SDL_GetError());
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

internal char *PathFromExecutable(const char *Filname) {
  const char *CurrentBasePath = SDL_GetBasePath();
  size_t CurrentPathSize = strlen(CurrentBasePath);

  // FIXME: this seems so wrong
  char *Result = (char *)SDL_calloc(strlen(Filname) + CurrentPathSize, 1);

  CatStrings(CurrentBasePath, Filname, Result);
  return Result;
}

SDL_AppResult SDL_AppInit(void **_AppState, int Argc, char **Argv) {
  srand(time(NULL)); // use current time as seed for random generator

  if (!SDL_Init(MyInitFlags))
    return SDL_APP_FAILURE;

  app_state *AppState = (app_state *)SDL_calloc(1, sizeof(app_state));
  if (!AppState)
    return SDL_APP_FAILURE;
  *_AppState = AppState;
  // default refresh 60 hz
  AppState->TargetRefreshRate = 60.0f;

  if (!SDL_CreateWindowAndRenderer("shapes-destroyer", SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE,
                                   &AppState->Window, &AppState->Renderer))
    return SDL_APP_FAILURE;

  SDL_DisplayID CurrentDisplayID = SDL_GetDisplayForWindow(AppState->Window);
  const SDL_DisplayMode *CurrentDisplayMode =
      SDL_GetCurrentDisplayMode(CurrentDisplayID);

  if (CurrentDisplayMode) {
    SDL_Log("the current refresh rate is %f", CurrentDisplayMode->refresh_rate);
    // TODO: add a debug build flag
    AppState->TargetRefreshRate = CurrentDisplayMode->refresh_rate / 2.0f;
  }

  AppState->TargetMsPerFrame = 1000.0f / AppState->TargetRefreshRate;

  GameCode = {0};

  // TODO: fix the output of the game code as library
#if HANDMADE_HERO
  GameCode.FullLibPath =
      PathFromExecutable("src/my_handmade_hero/libmy_handmade_hero.dylib");
#else
  GameCode.FullLibPath =
      PathFromExecutable("src/shapes_destroyer/libshapes_destroyer.dylib");
#endif

  MySDL_LoadGameCode(&GameCode);

  {
    // temporary solution, it should be done outside of here
    AppState->AudioBuffer.SamplesPerSecond = 48000;
    AppState->AudioBuffer.ToneHz = 260;
    AppState->AudioBuffer.ToneVolume = 100;
    AppState->AudioBuffer.Channels = 2;
    AppState->AudioBuffer.BytesPerSample =
        sizeof(float) * AppState->AudioBuffer.Channels;
    AppState->AudioBuffer.BufferSize = AppState->AudioBuffer.BytesPerSample *
                                       AppState->AudioBuffer.SamplesPerSecond;
    AppState->AudioBuffer.WavePeriod =
        AppState->AudioBuffer.SamplesPerSecond / AppState->AudioBuffer.ToneHz;

    AppState->AudioBuffer.Data =
        SDL_calloc(1, AppState->AudioBuffer.BufferSize);
  }
  if (!AppState->AudioBuffer.Data) {
    SDL_Log("Cannot initialize audio buffer %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!MySDL_InitSoundBuffer(&AppState->AudioBuffer))
    return SDL_APP_FAILURE;
  SDL_ResumeAudioStreamDevice(AppState->AudioBuffer.AudioStream);
  AppState->AudioIsPaused = false;

  MySDL_ResizeTexture(&AppState->BackBuffer, AppState->Renderer, SCREEN_WIDTH,
                      SCREEN_HEIGHT);

  AppState->GamePermanentStorage = MySDL_CreateMemoryArena(Megabytes(64));
  AppState->GameTransientStorage = MySDL_CreateMemoryArena(Gigabytes(4));

#if !HANDMADE_HERO
  AppState->GameEngine = GameCode.Bootstrap(AppState->GamePermanentStorage,
                                            AppState->GameTransientStorage);

  if (GameCode.JustReloaded && GameCode.ResetSystems) {
    GameCode.JustReloaded = false;
    GameCode.ResetSystems(AppState->GamePermanentStorage);
  }
#endif

  AppState->Controllers[0] = {0};
  AppState->Controllers[1] = {0};
  AppState->OldInput = &AppState->Controllers[0];
  AppState->NewInput = &AppState->Controllers[1];

  game_controller_input KeyboardController = {0};
  KeyboardController.IsActive = true;
  KeyboardController.IsAnalog = false;

  AppState->NewInput->Controllers[0] = KeyboardController;
  AppState->OldInput->Controllers[0] = KeyboardController;
  AppState->OldInput->ControllersCount = 1;
  AppState->NewInput->ControllersCount = 1;

  return SDL_APP_CONTINUE;
}

void SetButtonState(SDL_Event *Event, game_controller_button *Button) {
  Button->HalfTransitionCount = Event->key.repeat;
  Button->isDown = Event->key.down;
}

SDL_AppResult MySDL_HandleKeyEvent(SDL_Event *Event,
                                   sdl_controllers_mapping *Controllers,
                                   app_state *AppState) {
  game_controller_input *Keyboard = &Controllers->Controllers[0];

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
    if (Event->key.repeat > 0)
      break;

    if (!Event->key.down)
      break;

    if (AppState->AudioIsPaused) {
      SDL_ResumeAudioStreamDevice(AppState->AudioBuffer.AudioStream);
    } else {
      SDL_PauseAudioStreamDevice(AppState->AudioBuffer.AudioStream);
    }
    AppState->AudioIsPaused = !AppState->AudioIsPaused;
  } break;

  case SDL_SCANCODE_L: {
    // ignore repeated keys
    if (Event->key.repeat > 0)
      break;

    if (!Event->key.down)
      break;

    // we either record or play for now
    AppState->InputIsRecording = !AppState->InputIsRecording;
    AppState->InputIsPlaying = !AppState->InputIsRecording;
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
  game_controller_input *CurrentController =
      &Controllers->Controllers[ControllerIndex];

  CurrentController->IsActive = true;
}

game_controller_input *
ControllerByJoystickId(SDL_JoystickID JoystickId,
                       sdl_controllers_mapping *Controllers) {
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

  if (NormalizedValue < 0 && NormalizedValue > DeadZoneThreshold)
    return 0.0;

  if (NormalizedValue > 0 && NormalizedValue < DeadZoneThreshold)
    return 0.0;

  return NormalizedValue;
}

void MySDL_HandleButtonEvent(SDL_Event *Event,
                             sdl_controllers_mapping *Controllers) {
  // TODO: handle up and down with the transitions counter

  game_controller_input *CurrentController =
      ControllerByJoystickId(Event->jaxis.which, Controllers);
  if (!CurrentController)
    return;

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

SDL_AppResult SDL_AppEvent(void *_AppState, SDL_Event *Event) {
  app_state *AppState = (app_state *)_AppState;
  switch (Event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;

  case SDL_EVENT_JOYSTICK_ADDED: {
    MySDL_AddJoystick(Event, AppState->NewInput);
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
    game_controller_input *CurrentController =
        ControllerByJoystickId(Event->jaxis.which, AppState->NewInput);
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
    MySDL_HandleButtonEvent(Event, AppState->NewInput);
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_JOYSTICK_REMOVED: {
    // TODO: handle me
  }
    return SDL_APP_CONTINUE;

  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    return MySDL_HandleKeyEvent(Event, AppState->NewInput, AppState);
  }
  return SDL_APP_CONTINUE;
}

internal void MySDL_UpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer,
                                 sdl_offscreen_buffer *Buffer) {
  SDL_UpdateTexture(Buffer->Texture, 0, Buffer->Memory, Buffer->Pitch);

  SDL_RenderTexture(Renderer, Buffer->Texture, 0, 0);

  SDL_RenderPresent(Renderer);
}

internal void MySDL_HitTargetFramerate(app_state *AppState) {
  const u64 Now = SDL_GetTicks();
  u64 Elapsed = Now - AppState->LastTick;
  u32 WaitFor = AppState->TargetMsPerFrame - Elapsed;
#if DEBUG
  SDL_Log("Target %f - Time to Compute %" PRIu64 " - Wait for %" PRIu32,
          AppState->TargetMsPerFrame, Elapsed, WaitFor);
#endif
  SDL_Delay(WaitFor);
}

internal void SwapAndResetControllers(app_state *AppState) {
  sdl_controllers_mapping *Temp = AppState->OldInput;
  AppState->OldInput = AppState->NewInput;
  AppState->NewInput = Temp;

  game_controller_input ZeroKeyboard = {0};
  ZeroKeyboard.IsActive = true;
  ZeroKeyboard.IsAnalog = false;
  AppState->NewInput->Controllers[0] = ZeroKeyboard;

  game_controller_input *OldKeyboardController =
      &AppState->OldInput->Controllers[0];
  game_controller_input *NewKeyboardController =
      &AppState->NewInput->Controllers[0];

  for (int ButtonIndex = 0;
       ButtonIndex < ArraySize(OldKeyboardController->Buttons); ButtonIndex++) {
    NewKeyboardController->Buttons[ButtonIndex].isDown =
        OldKeyboardController->Buttons[ButtonIndex].isDown;
  }
}
internal void MySDL_RecordInput(app_state *AppState) {
  // code the record the input, take the newinput and put in a file
  sdl_controllers_mapping *Input = AppState->NewInput;
  if (!AppState->RecordFilename) {
    AppState->RecordFilename = PathFromExecutable("loop.rec");
  }

  if (!AppState->RecordStream) {
    AppState->RecordStream =
        SDL_IOFromFile((const char *)AppState->RecordFilename, "wb+");
  } else {
    // SDL_IOStatus RecordStatus = SDL_GetIOStatus(AppState->RecordStream);
  }

  if (!AppState->RecordStream) {
    SDL_Log("Error while opening the record file `%s` stream %s",
            AppState->RecordFilename, SDL_GetError());
  }

  // if it's really slow improve it, otherwise leave it is debug code anyway
  // SDL_SeekIO(RecordIOStream, AppState->RecordOffsetFromStart,
  // SDL_IO_SEEK_SET);
  size_t BytesWritten = SDL_WriteIO(AppState->RecordStream, Input,
                                    sizeof(sdl_controllers_mapping));
  AppState->RecordOffsetFromStart += BytesWritten;
  // SDL_CloseIO(RecordIOStream);

  if (BytesWritten != sizeof(sdl_controllers_mapping)) {
    SDL_Log("Written %lu Expected %lu", BytesWritten,
            sizeof(sdl_controllers_mapping));
  }
}

internal void MySDL_PlaybackInput(app_state *AppState) {
  sdl_controllers_mapping *Input = AppState->NewInput;
  if (!AppState->PlaybackFilename) {
    AppState->PlaybackFilename = PathFromExecutable("loop.rec");
  }

  if (!AppState->PlaybackStream) {
    AppState->PlaybackStream =
        SDL_IOFromFile((const char *)AppState->PlaybackFilename, "rb");
  }

  SDL_IOStream *Stream = AppState->PlaybackStream;
  if (!Stream) {
    SDL_Log("Error while opening the record file `%s` stream %s",
            AppState->PlaybackFilename, SDL_GetError());
  }

#if 0
  if (SDL_SeekIO(Stream, AppState->PlaybackOffsetFromStart, SDL_IO_SEEK_SET) !=
      -1) {
#endif

  size_t BytesRead = SDL_ReadIO(Stream, Input, sizeof(sdl_controllers_mapping));
  AppState->PlaybackOffsetFromStart += BytesRead;

  if (BytesRead != sizeof(sdl_controllers_mapping)) {
    SDL_Log("Read %lu Expected %lu Offset %lu", BytesRead,
            sizeof(sdl_controllers_mapping), AppState->PlaybackOffsetFromStart);
    // read from the beginning
    AppState->PlaybackOffsetFromStart = 0;
    SDL_SeekIO(AppState->PlaybackStream, AppState->PlaybackOffsetFromStart,
               SDL_IO_SEEK_SET);
  }

#if 0
  } else {
    SDL_Log("error while seeking %s", SDL_GetError());
  }

  SDL_CloseIO(Stream);
#endif
}

SDL_AppResult SDL_AppIterate(void *_AppState) {
  app_state *AppState = (app_state *)_AppState;
  u64 Now = SDL_GetTicks();
  // we want the DeltaTime in seconds and the value returned by SDL_GetTicks are
  // milliseconds
  f32 DeltaTime = (f32)((f64)(Now - AppState->LastTick) / 1000.0f);
  AppState->LastTick = Now;

  // reset the transient storage, without actually zeroing the memory
  AppState->GameTransientStorage->used = 0;

  // FIXME: there is a bug once we toggle the recording after the first time it
  // keeps playing back the previous input making impossible to controll the
  // square I haven't debugged it yet
  if (AppState->InputIsRecording) {
    MySDL_RecordInput(AppState);
  }

  if (AppState->InputIsPlaying) {
    MySDL_PlaybackInput(AppState);
  }

  if (GameCode.UpdateAndRender) {
    game_audio_buffer AudioBuffer = {0};
    AudioBuffer.Data = AppState->AudioBuffer.Data;
    AudioBuffer.BufferSize = AppState->AudioBuffer.BufferSize;
    AudioBuffer.BytesPerSample = AppState->AudioBuffer.BytesPerSample;
    AudioBuffer.SamplesPerSecond = AppState->AudioBuffer.SamplesPerSecond;
    AudioBuffer.Channels = AppState->AudioBuffer.Channels;
    AudioBuffer.ToneHz = AppState->AudioBuffer.ToneHz;
    AudioBuffer.WavePeriod = AppState->AudioBuffer.WavePeriod;

    game_offscreen_buffer BackBuffer = {0};
    BackBuffer.Height = AppState->BackBuffer.Height;
    BackBuffer.Memory = AppState->BackBuffer.Memory;
    BackBuffer.Pitch = AppState->BackBuffer.Pitch;
    BackBuffer.Width = AppState->BackBuffer.Width;
    BackBuffer.BytesPerPixel = AppState->BackBuffer.BytesPerPixel;

    GameCode.UpdateAndRender(AppState->GamePermanentStorage, DeltaTime,
                             AppState->NewInput->Controllers, &BackBuffer,
                             &AudioBuffer);
  }

  MySDL_FillSoundBuffer(&AppState->AudioBuffer);
  MySDL_UpdateWindow(AppState->Window, AppState->Renderer,
                     &AppState->BackBuffer);

  MySDL_LoadGameCode(&GameCode);
#if !HANDMADE_HERO
  if (GameCode.JustReloaded && GameCode.ResetSystems) {
    GameCode.JustReloaded = false;
    GameCode.ResetSystems(AppState->GamePermanentStorage);
  }
#endif

  SwapAndResetControllers(AppState);
  // with DeltaTime we should be FPS independent but let's leave this here in
  // order to not compute stuff for nothing
  MySDL_HitTargetFramerate(AppState);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *AppState, SDL_AppResult Result) {
  if (AppState != NULL) {
    app_state *As = (app_state *)AppState;
    SDL_PauseAudioStreamDevice(As->AudioBuffer.AudioStream);
#if !HANDMADE_HERO
    if (GameCode.Destroy) {
      GameCode.Destroy(As->GameEngine);
    }
#endif
    SDL_free(As->AudioBuffer.Data);
    SDL_free(As->GameTransientStorage);
    SDL_free(As->GamePermanentStorage);
    SDL_free(As);
  }
}
