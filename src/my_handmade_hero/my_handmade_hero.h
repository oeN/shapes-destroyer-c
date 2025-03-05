#pragma once

#include "../constants.h"
#include "../memory.h"
#include "../types.h"

struct game_offscreen_buffer {
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  u8 BytesPerPixel;
};

struct game_audio_buffer {
  void *Data;
  u8 Channels;
  u32 BufferSize;
  int BytesPerSample;
  int SamplesPerSecond;
  int ToneHz;
  int ToneVolume;
  int WavePeriod;
};

struct game_controller_button {
  int HalfTransitionCount;
  bool isDown;
};

struct game_controller_input {
  bool IsActive;
  bool IsAnalog;
  float StickX;
  float StickY;

  union {
    game_controller_button Buttons[12];
    struct {
      game_controller_button MoveUp;
      game_controller_button MoveRight;
      game_controller_button MoveDown;
      game_controller_button MoveLeft;

      game_controller_button ButtonSouth;
      game_controller_button ButtonEast;
      game_controller_button ButtonNorth;
      game_controller_button ButtonWest;

      game_controller_button ShoulderLeft;
      game_controller_button ShoulderRight;

      game_controller_button Start;
      game_controller_button Back;
    };
  };
};

struct game_state {
  vec2 PlayerPosition;
  f32 CurrentAngle;
};

#define GAME_UPDATE_AND_RENDER(name)                                           \
  void(name)(memory_arena * PermanentStorage, f32 deltaTime,                   \
             game_controller_input Controllers[MAX_N_CONTROLLERS],             \
             game_offscreen_buffer * BackBuffer,                               \
             game_audio_buffer * AudioBuffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
