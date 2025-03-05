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

void DrawLine(game_offscreen_buffer *Buffer, vec2 Start, vec2 End, u32 Color) {
  // TODO: Makes sure the Start is the minimum one
  BoundVec2(Buffer, &Start);
  BoundVec2(Buffer, &End);

  i32 MinX = (i32)Start.x;
  i32 MinY = (i32)Start.y;

  i32 MaxX = (i32)End.x;
  i32 MaxY = (i32)End.y;

  i32 DeltaX = MaxX - MinX;
  i32 DeltaY = MaxY - MinY;

  f32 Slope;
  if (DeltaX && DeltaY)
    Slope = (f32)DeltaY / (f32)DeltaX;
  else
    Slope = 0;

  f32 CumulativeSlope = (f32)(DeltaX > 0 ? MinY : MaxY);

  i32 Direction = DeltaX > 0 ? 1 : -1;

  i32 Min, Max;
  if (Direction == 1) {
    Min = MinX;
    Max = MaxX;
  } else {
    Min = MaxX;
    Max = MinX;
  }

  bool EqualMinMax = Min == Max;
  if (Min == Max) {
    // make it print at least 1px
    Max = Min + 1;

    // printf("We're on the same column new min, max (%d, %d) - CumulativeSlope
    // "
    //        "%d - Slope %f\n",
    //        Min, Max, (i32)CumulativeSlope, Slope);
  }

  if (EqualMinMax) {
    i32 Min = MinY < MaxY ? MinY : MaxY;
    i32 Max = MinY < MaxY ? MaxY : MinY;
    i32 X = MinX;

    for (i32 Y = Min; Y < Max; Y++) {
      u32 *Pixel = PixelFromBuffer(Buffer, X, Y);
      *Pixel = Color;
    }
  } else {
    for (i32 X = Min; X < Max; X++) {
      i32 Y = (i32)CumulativeSlope;

      u32 *Pixel = PixelFromBuffer(Buffer, X, Y);
      *Pixel = Color;

      CumulativeSlope += Slope;
    }
  }
}

void DrawPoint(game_offscreen_buffer *Buffer, vec2 Point, u32 Color) {
  u32 *Pixel = PixelFromBuffer(Buffer, Point.x, Point.y);
  *Pixel = Color;
}

void DrawShape(game_offscreen_buffer *BackBuffer, vec2 Center, f32 Radius,
               u32 NumberOfSegments, u32 Color) {
  f32 AngleStep = 360.0f / (f32)NumberOfSegments;
  f32 CurrentAngle = 0;

  vec2 Prev = {0};

  while (CurrentAngle < 360.0f) {
    vec2 Point = Center + (vec2FromAngle(CurrentAngle) * Radius);
    DrawPoint(BackBuffer, Point, 0xFFFF00FF);
    if (Prev.x != 0 && Prev.y != 0) {
      DrawLine(BackBuffer, Prev, Point, Color);
    }

    CurrentAngle += AngleStep;
    Prev = Point;
  }

  vec2 Point = Center + (vec2FromAngle(0) * Radius);
  DrawLine(BackBuffer, Prev, Point, Color);

#if 0
  for (int i = 0; i < NumberOfSegments; i++) {
  }

  vec2 Start = *Points[0];

  vec2 End = vec2FromAngle(CurrentAngle - AngleStep) * Radius;
  End += Center;
  DrawLine(BackBuffer, Start, End, Color);
#endif
}

struct game_state {
  vec2 PlayerPosition;
};

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

  DrawShape(BackBuffer, StartLine, 50.0f, 8, 0xFF0000FF);
}
