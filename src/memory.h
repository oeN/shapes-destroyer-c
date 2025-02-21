#pragma once

#include <string.h>

#include "base_types.h"

typedef struct MemoryArena {
  memory_size totalSize;
  memory_size used;
  u8 *startAddress;
} memory_arena;

#define Kilobytes(n) 1024 * n
#define Megabytes(n) 1024 * 1024 * n

#define pushStruct(Arena, type) (type *)_pushSize(Arena, sizeof(type))
#define pushString(Arena, string)                                              \
  (char *)_pushSize(Arena, sizeof(char) * strlen(string))
#define pushSizeTimes(Arena, type, times)                                      \
  (type *)_pushSize(Arena, sizeof(type) * times)
#define getNodeValue(Node, type) (type *)_getNodeValue(Node)

memory_arena *bootstrapArena(memory_size totalSize);
void initArena(memory_arena *arena, memory_size totalSize);
void freeArena(memory_arena *arena);
void resetArena(memory_arena *arena, bool zeroIt);

void pushToLinkedList(memory_arena *arena, linked_list_node **firstOrCurrent,
                      linked_list_node *next);
linked_list_node *popFromLinkedList(linked_list_node *firstOrCurrent);

void *_pushSize(memory_arena *arena, memory_size sizeToPush);
void *_getNodeValue(linked_list_node *node);
