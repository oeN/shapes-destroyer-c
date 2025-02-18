#pragma once

#include "base_types.h"

typedef struct MemoryArena {
  memory_size totalSize;
  memory_size used;
  u8 *startAddress;
} memory_arena;

#define Kilobytes(n) 1024 * n
#define Megabytes(n) 1024 * 1024 * n

#define pushStruct(Arena, type) (type *)_pushSize(Arena, sizeof(type))
#define pushSizeTimes(Arena, type, times)                                      \
  (type *)_pushSize(Arena, sizeof(type) * times)

void initArena(memory_arena *arena, memory_size totalSize);
void freeArena(memory_arena *arena);

void *_pushSize(memory_arena *arena, memory_size sizeToPush);
