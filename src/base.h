#pragma once
#include <stddef.h>

#include "types.h"

unsigned long hash(unsigned char *str);
void increaseArray(void **array, size_t elementSize, int *currentSize);
int randomClamped(int min, int max);
void freeList(Node *node);
