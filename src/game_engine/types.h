#pragma once

#include "../base_types.h"

typedef struct GameOffscreenBuffer {
  void *memory;
  int width;
  int height;
  int pitch;
} game_offscreen_buffer;

struct wayne_audio_buffer {
  void *Data;
  u8 Channels;
  u32 BufferSize;
  int BytesPerSample;
  int SamplesPerSecond;
  int ToneHz;
  int ToneVolume;
  int WavePeriod;
};

struct wayne_controller_button {
  bool isDown;
};

struct wayne_controller_input {
  // TODO: do you need this?
  bool isActive;
  bool isAnalog;
  float StickX;
  float StickY;
  union {
    struct {
      wayne_controller_button ButtonSouth;
      wayne_controller_button ButtonEast;
      wayne_controller_button ButtonNorth;
      wayne_controller_button ButtonWest;
      wayne_controller_button ShoulderLeft;
      wayne_controller_button ShoulderRight;
    };
    wayne_controller_button Buttons[6];
  };
};
