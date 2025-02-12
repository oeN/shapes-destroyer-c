#include <stdlib.h> 

#include "base.h"
#include <SDL3/SDL.h>

void increaseArray(void **array, size_t elementSize, int *currentSize)
{
  *currentSize += 1;
  *array = SDL_realloc(*array, (*currentSize) * elementSize);
  if (*array == NULL)
  {
    SDL_Log("Failed to reallocate memory");
  }
}

unsigned long hash(unsigned char *str) {
  unsigned long hash = 5381;
  int c;
  
  while((c = *str++))
    hash = ((hash << 5) + hash) + c;

  return hash;
}

int randomClamped(int min, int max)
{
  return rand()/((RAND_MAX + (unsigned)min)/max);
}
