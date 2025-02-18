#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "memory.h"

void debugArena(memory_arena *arena) {
  return;
  printf("---- DEBUG ARENA - USED: %lu\n", arena->used);
  for (int i = 0; i < arena->used; i++) {
    int c = *(arena->startAddress + i);
    printf("%d", c);
  }
  printf("\n----------\n");
}

void initArena(memory_arena *arena, memory_size totalSize) {
  arena->startAddress = malloc(totalSize);
  arena->totalSize = totalSize;
  arena->used = 0;
  memset(arena->startAddress, 0, totalSize);
  debugArena(arena);
}

void freeArena(memory_arena *arena) { free(arena->startAddress); }

void *_pushSize(memory_arena *arena, memory_size sizeToPush) {
  void *result = 0;

  if ((arena->used + sizeToPush) > arena->totalSize) {
    printf("Too much memory");
    // TODO: find a better way to handle this case
    return nullptr;
  }

  result = arena->startAddress + arena->used;
  arena->used += sizeToPush;

  debugArena(arena);

  return result;
}
