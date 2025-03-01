#pragma once

#include "../base_types.h"

typedef struct GameOffscreenBuffer {
  void *Memory;
  int Width;
  int Height;
  int Pitch;
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
  // TODO: extract to knowledge base
  // to count how many time the button changed state during that frame or
  // interval we pull the joystick data or event in which the joystick data are
  // being fired by the platform
  //
  // I got this by the Handmade Hero series and since is still fully clear to
  // me, probably because I'm using SDL instead of lower level platform
  // functions, with this count we can check how fast a button as been pressed
  // and make the game react in the correct way
  //
  // for example if we want to dash in a certain direction we could map that
  // dash to a certain amount of transitions
  int HalfTransitionCount;
  bool isDown;
};

struct wayne_controller_input {
  // TODO: do you need this?
  bool IsActive;
  bool IsAnalog;
  float StickX;
  float StickY;

  union {
    wayne_controller_button Buttons[12];
    struct {
      wayne_controller_button MoveUp;
      wayne_controller_button MoveRigth;
      wayne_controller_button MoveDown;
      wayne_controller_button MoveLeft;

      wayne_controller_button ButtonSouth;
      wayne_controller_button ButtonEast;
      wayne_controller_button ButtonNorth;
      wayne_controller_button ButtonWest;

      wayne_controller_button ShoulderLeft;
      wayne_controller_button ShoulderRight;

      wayne_controller_button Start;
      wayne_controller_button Back;
    };
  };
};
