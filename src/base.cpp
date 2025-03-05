#include <stdlib.h>

#include "base.h"

unsigned long hash(unsigned char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;

  return hash;
}

int randomClamped(int min, int max) { return rand() % (max + 1 - min) + min; }
