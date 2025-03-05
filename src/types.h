#pragma once

#include <stddef.h>
#include <stdint.h>

// static could means different things based on where it's used
// internal: at a function level it means that function can be accessed only
// from function in the same file
#define internal static
// local_persist: a variable inside a block that should be freed is persisted
// for future run of the same block
#define local_persist static
// glabal_variable: a variable is global and can be used from every function
// without getting it from params
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t uint8;   // can hold from 0 to 255
typedef uint16_t uint16; // can hold from 0 to 65535
typedef uint32_t uint32; // up to 4 billion
typedef uint64_t uint64;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef float f32;
typedef double f64;

typedef size_t memory_size;

typedef struct Node linked_list_node;
struct Node {
  linked_list_node *next;
  void *value;
};

union vec2 {
  f32 v[2];
  struct {
    f32 x, y;
  };
  struct {
    f32 w, h;
  };

  void operator+=(vec2 rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
  }

  vec2 operator+(vec2 rhs) {
    vec2 result = *this;
    result.x += rhs.x;
    result.y += rhs.y;
    return result;
  }

  vec2 operator-(vec2 rhs) {
    vec2 result = *this;
    result.x -= rhs.x;
    result.y -= rhs.y;
    return result;
  }

  vec2 operator*(f32 rhs) {
    vec2 result = *this;
    result.x *= rhs;
    result.y *= rhs;
    return result;
  }
};

union vec2i {
  f32 v[2];
  struct {
    f32 x, y;
  };
  struct {
    f32 w, h;
  };
};

typedef union Vec4 {
  f32 v[4];
  struct {
    f32 r, g, b, a;
  };
} vec4;
