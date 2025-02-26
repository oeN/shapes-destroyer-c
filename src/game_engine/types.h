#pragma once

typedef struct GameOffscreenBuffer {
  void *memory;
  int width;
  int height;
  int pitch;
} game_offscreen_buffer;
