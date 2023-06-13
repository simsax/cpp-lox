#ifndef clox_arena_h
#define clox_arena_h

#include "common.h"

#define DEBUG 1

typedef struct header {
    int size;
#if DEBUG
    int magic;
#endif
    struct header* next;
} Header;

typedef struct node {
    Header header;
    uint8_t data[];
} Node;

typedef struct arena {
    size_t size;
    size_t current;
    Header* free;
    Header* allocated;
    struct arena* next;
} Arena;

Arena* arena_create();

void* arena_malloc(Arena* arena, size_t size);

void* arena_malloc_block(Arena* arena, size_t size);

void* arena_free_block(Arena* arena, void* ptr);

// // assume it contains only one type of data, starting from the beginning of the data array
void* arena_realloc(Arena** arena, size_t size, void* ptr);

void arena_free(Arena* arena);

// debug
void arena_print(Arena* arena);

#endif