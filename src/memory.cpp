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

memory_arena *bootstrapArena(memory_size totalSize) {
  memory_arena *arena = (memory_arena *)malloc(sizeof(memory_arena));
  initArena(arena, totalSize);
  return arena;
}

void initArena(memory_arena *arena, memory_size totalSize) {
  arena->startAddress = (u8 *)malloc(totalSize);
  arena->totalSize = totalSize;
  arena->used = 0;
  memset(arena->startAddress, 0, totalSize);
  debugArena(arena);
}

void resetArena(memory_arena *arena, bool zeroIt) {
  arena->used = 0;
  if (zeroIt)
    memset(arena->startAddress, 0, arena->totalSize);
}

void freeArena(memory_arena *arena) {
  free(arena->startAddress);
  free(arena);
}

void pushToLinkedList(memory_arena *arena, linked_list_node **firstOrCurrent,
                      linked_list_node *next) {
  if (*firstOrCurrent == NULL) {
    *firstOrCurrent = pushStruct(arena, linked_list_node);
  }

  linked_list_node *properTarget = *firstOrCurrent;
  while (properTarget->next != NULL) {
    properTarget = properTarget->next;
  }
  properTarget->next = next;
  next->next = NULL;
}

linked_list_node *popFromLinkedList(linked_list_node *firstOrCurrent) {
  if (!firstOrCurrent)
    return NULL;

  if (!firstOrCurrent->next)
    return NULL;

  linked_list_node *node = firstOrCurrent;
  linked_list_node *prev = NULL;
  while (node->next) {
    prev = node;
    node = node->next;
  }

  // remove the link to the last node
  prev->next = NULL;

  // return the last one
  return node;
}

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

void *_getNodeValue(linked_list_node *node) { return node->value; }
