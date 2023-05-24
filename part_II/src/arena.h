#ifndef clox_arena_h
#define clox_arena_h

#include "common.h"

/*
https://github.com/thejefflarson/arena
*/

typedef struct arena {
    size_t size;
    size_t current;
    uint8_t* data;
    struct arena* next;
} Arena;

Arena* arena_create();

void* arena_malloc(Arena* arena, size_t size);

// force allocation into a new block
void* arena_malloc_block(Arena* arena, size_t size);

// assume it contains only one type of data, starting from the beginning of the data array
void* arena_realloc(Arena** arena, size_t size, void* ptr);

void arena_free(Arena* arena);
void arena_free_block(Arena* arena, void* ptr);

// debug
void arena_print(Arena* arena);

#endif