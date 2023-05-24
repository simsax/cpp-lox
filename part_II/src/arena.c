#include "arena.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 4096

static Arena* s_arena_create(size_t size)
{
    Arena* arena = malloc(sizeof *arena);
    if (!arena)
        return NULL;
    arena->current = 0;
    arena->size = size;
    arena->data = malloc(sizeof *(arena->data) * size);
    if (!arena->data) {
        free(arena);
        return NULL;
    }
    arena->next = NULL;
    return arena;
}

Arena* arena_create() { return s_arena_create(BLOCK_SIZE); }

void* arena_malloc(Arena* arena, size_t size)
{
    Arena* last_arena = arena;
    while (arena != NULL) {
        if (arena->size - arena->current >= size) {
            size_t start_index = arena->current;
            arena->current += size;
            return arena->data + start_index;
        }
        last_arena = arena;
        arena = arena->next;
    }
    size_t arena_size = size < BLOCK_SIZE ? BLOCK_SIZE : size;
    Arena* new_arena = s_arena_create(arena_size);
    if (new_arena == NULL)
        return NULL;
    last_arena->next = new_arena;
    new_arena->current += size;
    return new_arena->data;
}

void* arena_malloc_block(Arena* arena, size_t size)
{
    Arena* last_arena = arena;
    while (arena != NULL) {
        last_arena = arena;
        arena = arena->next;
    }
    size_t arena_size = size < BLOCK_SIZE ? BLOCK_SIZE : size;
    Arena* new_arena = s_arena_create(arena_size);
    if (new_arena == NULL)
        return NULL;
    last_arena->next = new_arena;
    new_arena->current += size;
    return new_arena->data;
}

void* arena_realloc(Arena** arena, size_t size, void* ptr)
{
    Arena* prev_arena = *arena;
    Arena* cur_arena = *arena;
    bool head_insertion = false;
    while (cur_arena != NULL && cur_arena->data != ptr) {
        prev_arena = cur_arena;
        cur_arena = cur_arena->next;
    }
    if (cur_arena == NULL)
        return NULL;
    if (prev_arena == cur_arena)
        head_insertion = true;
    if (cur_arena->size >= size) {
        cur_arena->current = size;
        return cur_arena->data;
    }
    size_t arena_size = size < BLOCK_SIZE ? BLOCK_SIZE : size;
    Arena* new_arena = s_arena_create(arena_size);
    if (new_arena == NULL)
        return NULL;
    memcpy(new_arena->data, cur_arena->data, cur_arena->current);
    if (!head_insertion)
        prev_arena->next = new_arena;
    new_arena->next = cur_arena->next;
    new_arena->current += size;
    free(cur_arena->data);
    free(cur_arena);
    if (head_insertion)
        *arena = new_arena;
    return new_arena->data;
}

void arena_free(Arena* arena)
{
    while (arena != NULL) {
        Arena* cur_arena = arena;
        arena = arena->next;
        free(cur_arena->data);
        free(cur_arena);
    }
}

void arena_free_block(Arena* arena, void* ptr)
{
    Arena* prev_arena = arena;
    while (arena != NULL && arena->data != ptr) {
        prev_arena = arena;
        arena = arena->next;
    }
    if (arena == NULL)
        return;
    prev_arena->next = arena->next;
    free(arena->data);
    free(arena);
}

void arena_print(Arena* arena)
{
    int arena_num = 0;
    while (arena != NULL) {
        arena_num++;
        printf("> ARENA %d [size: %zu, current: %zu]\n", arena_num, arena->size, arena->current);
        arena = arena->next;
    }
}
