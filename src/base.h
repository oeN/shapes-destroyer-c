#pragma once

// simple macro to assert things during development
// this shouldn't be used in production
//
// I shouldn't need to explain but I'll do anyway if the given expression is not
// true we try to write to the pointer of 0 and it should result in a crash
// letting us spot the bug immediately
#define Assert(Expression)                                                     \
  if (!(Expression)) {                                                         \
    *(int *)0 = 0;                                                             \
  }

#define ArrayCount(Array) sizeof(Array) / sizeof(Array[0])
#define ArraySize(Array) ArrayCount(Array)

unsigned long hash(unsigned char *str);
int randomClamped(int min, int max);
