#include "my_handmade_hero.h"
#include "../mymath.cpp"
#include "../mymath.h"
#include "../types.h"

#include <stdio.h>

void BoundVec2(game_offscreen_buffer *Buffer, vec2 *VecToBound) {
  // it makes sure the given vector is inside the buffer bounds
  VecToBound->x = Clamp(f32, VecToBound->x, 0, Buffer->Width);
  VecToBound->y = Clamp(f32, VecToBound->y, 0, Buffer->Height);
}

void DrawRectangle(game_offscreen_buffer *Buffer, vec2 Position,
                   vec2 Dimensions, u32 Color) {

  vec2 Max = Position + Dimensions;

  i32 MinX = (i32)Clamp(f32, Position.x, 0, Buffer->Width);
  i32 MinY = (i32)Clamp(f32, Position.y, 0, Buffer->Height);
  i32 MaxX = (i32)Clamp(f32, Max.x, 0, Buffer->Width);
  i32 MaxY = (i32)Clamp(f32, Max.y, 0, Buffer->Height);

  uint8 *Row = ((uint8 *)Buffer->Memory + Buffer->Pitch * MinY +
                Buffer->BytesPerPixel * MinX);

  for (int Y = MinY; Y < MaxY; ++Y) {
    uint32 *Pixel = (uint32 *)Row;

    for (int X = MinX; X < MaxX; ++X) {
      *Pixel++ = Color;
    }

    Row += Buffer->Pitch;
  }
}

u32 *PixelFromBuffer(game_offscreen_buffer *Buffer, i32 X, i32 Y) {
  return (u32 *)((u8 *)Buffer->Memory + Buffer->Pitch * Y +
                 Buffer->BytesPerPixel * X);
}

inline void DrawPoint(game_offscreen_buffer *Buffer, vec2 Point, u32 Color) {
  u32 *Pixel = PixelFromBuffer(Buffer, Point.x, Point.y);
  *Pixel = Color;
}

void DrawLine(game_offscreen_buffer *Buffer, vec2 Start, vec2 End, u32 Color) {
  BoundVec2(Buffer, &Start);
  BoundVec2(Buffer, &End);

  i32 MinX = (i32)Start.x;
  i32 MinY = (i32)Start.y;

  i32 MaxX = (i32)End.x;
  i32 MaxY = (i32)End.y;

  i32 DeltaX = MaxX - MinX;
  i32 DeltaY = MaxY - MinY;

  f32 Slope = 0;
  if (DeltaX != 0 && DeltaY != 0) {
    Slope = (f32)DeltaY / (f32)DeltaX;
  }

  i32 Min = GetMin(MinX, MaxX);
  i32 Max = GetMax(MinX, MaxX);

  // TODO: fix this function, I mean it works it prints lines in some way but it
  // feels junky with these edge cases etc.

  // 2 pixels tollerance in order to draw a vertical line
  if (Min == Max || (Min + 1) == Max || (Min + 2) == Max) {
    i32 Min = GetMin(MinY, MaxY);
    i32 Max = GetMax(MinY, MaxY);
    i32 X = MinX;

    // u32 *Pixel = PixelFromBuffer(Buffer, X, Min);
    for (i32 Y = Min; Y < Max; Y++) {
      // *Pixel = Color;
      // this is because the buffer is an array of u8 pointer that points to a
      // u32
      // Pixel = (u32 *)((u8 *)Pixel + Buffer->Pitch);

      // NOTE: for now I'll stick with this more convenient code even if the one
      // above should be faster
      DrawPoint(Buffer, (vec2){X, Y}, Color);
    }
  } else {
    f32 YOffset = (f32)(DeltaX > 0 ? MinY : MaxY);
    for (i32 X = Min; X < Max; X++) {
      i32 Y = (i32)YOffset;

      DrawPoint(Buffer, (vec2){X, Y}, Color);

      YOffset += Slope;
    }
  }
}

void DrawShape(game_offscreen_buffer *BackBuffer, f32 StartingAngle,
               vec2 Center, f32 Radius, u32 NumberOfSegments, u32 Color) {
  f32 AngleStep = 360.0f / (f32)NumberOfSegments;
  f32 CurrentAngle = StartingAngle;
  f32 LimitAngle = 360.0f + StartingAngle;

  vec2 Prev = {0};

  u32 DebugColor = 0xFFFF00FF;
  DrawPoint(BackBuffer, Center, DebugColor);

  while (CurrentAngle < LimitAngle) {
    vec2 Point = Center + (vec2FromAngle(CurrentAngle) * Radius);

    // debug line
    DrawLine(BackBuffer, Center, Point, DebugColor);

    if (Prev.x != 0 && Prev.y != 0) {
      DrawLine(BackBuffer, Prev, Point, Color);
    }

    CurrentAngle += AngleStep;
    Prev = Point;
  }

  vec2 Point = Center + (vec2FromAngle(StartingAngle) * Radius);
  DrawLine(BackBuffer, Prev, Point, Color);
}

void DrawShape(game_offscreen_buffer *BackBuffer, vec2 Center, f32 Radius,
               u32 NumberOfSegments, u32 Color) {

  DrawShape(BackBuffer, 0.0f, Center, Radius, NumberOfSegments, Color);
}

extern "C" GAME_UPDATE_AND_RENDER(UpdateAndRender) {
  vec2 Position = {0};
  vec2 Dimensions = {(f32)BackBuffer->Width, (f32)BackBuffer->Height};

  game_controller_input Keyboard = Controllers[0];
  game_state *GameState = (game_state *)PermanentStorage->startAddress;

  vec2 PlayerDelta = {0};
  if (Keyboard.MoveDown.isDown) {
    PlayerDelta.y = 1.0;
  }

  if (Keyboard.MoveUp.isDown) {
    PlayerDelta.y = -1.0;
  }

  if (Keyboard.MoveLeft.isDown) {
    PlayerDelta.x = -1.0;
  }

  if (Keyboard.MoveRight.isDown) {
    PlayerDelta.x = 1.0;
  }
  GameState->PlayerPosition += PlayerDelta * 128.0f * deltaTime;

  DrawRectangle(BackBuffer, Position, Dimensions, 0xFF87ceeb);

  vec2 PlayerSize = (vec2){50.0f, 50.0f};
  DrawRectangle(BackBuffer, GameState->PlayerPosition, PlayerSize, 0xFFFFFFFF);

  vec2 HalfSize = PlayerSize * 0.5f;
  vec2 StartLine = GameState->PlayerPosition + HalfSize;
  vec2 EndLine = StartLine + (vec2){HalfSize.x, 1.0f};
  DrawLine(BackBuffer, StartLine, EndLine, 0xFFFF0000);

  GameState->CurrentAngle += 30.0f * deltaTime;

  DrawShape(BackBuffer, GameState->CurrentAngle, (vec2){400.0f, 100.0f}, 50.0f,
            8, 0xFF0000FF);
  DrawShape(BackBuffer, GameState->CurrentAngle, (vec2){400.0f, 200.0f}, 50.0f,
            3, 0xFF0000FF);
  DrawShape(BackBuffer, GameState->CurrentAngle, (vec2){400.0f, 300.0f}, 50.0f,
            4, 0xFF0000FF);
  DrawShape(BackBuffer, GameState->CurrentAngle, (vec2){400.0f, 400.0f}, 50.0f,
            5, 0xFF0000FF);
}
