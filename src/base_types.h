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

typedef uint8_t uint8;   // can hold from 0 to 255
typedef uint16_t uint16; // can hold from 0 to 65535
typedef uint32_t uint32; // up to 4 billion
typedef uint64_t uint64;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef size_t memory_size;

typedef struct Node linked_list_node;
struct Node {
  linked_list_node *next;
  void *value;
};
