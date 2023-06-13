#include "arena.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 4096

#define GET_HEADER(block) ((Header*)(block - sizeof(Header)))

static Arena* s_arena_create(size_t size)
{
    Arena* arena = malloc(sizeof *arena);
    if (!arena)
        return NULL;
    arena->current = 0;
    arena->size = size;
    arena->free = malloc(size);
    arena->free->header.magic = 420;
    arena->free->header.size = size;
    arena->free->header.next = NULL;
    arena->allocated = NULL;
    if (!arena->free) {
        free(arena);
        return NULL;
    }
    arena->next = NULL;
    return arena;
}

Arena* arena_create() { return s_arena_create(BLOCK_SIZE); }

static Header* find_block(Arena* arena, size_t size)
{
    Header* node = (Header*)arena->free;
    Header* prev = node;
    while (node != NULL) {
        if (node->size >= size) {
            // pop node from free list
            if (prev == node)
                arena->free = node->next;
            else
                prev->next = node->next;

            return node;
        }
        prev = node;
        node = node->next;
    }
    return NULL;
}

void* allocate_block(Arena* arena, size_t size)
{
    Node* block;
    if ((block = (Node*)find_block(arena, size)) != NULL) {
        uint8_t* next_free_block = block->data + size;
        Node next_free_node
            = { .header = { .magic = 11, .size = block->header.size - size, .next = arena->free } };
        // now insertion in head, TODO: sort by memory address
        memcpy(next_free_block, &next_free_node, sizeof(Header));
        arena->free = (Header*)next_free_block;
        block->header =.size = size, .magic = 69, .next = arena->allocated
    };
    arena->allocated = (Header*)block;

    return block->data;
}
}

void* arena_malloc(Arena* arena, size_t size)
{
    Arena* last_arena = arena;
    while (arena != NULL) {
        void* block;
        if (block = allocate_block(arena, size))
            return block;
        last_arena = arena;
        arena = arena->next;
    }
    size_t arena_size = size < BLOCK_SIZE ? BLOCK_SIZE : size;
    Arena* new_arena = s_arena_create(arena_size);
    if (new_arena == NULL)
        return NULL;
    last_arena->next = new_arena;
    return allocate_block(new_arena, size);
}

// void* arena_realloc(Arena** arena, size_t size, void* ptr)
// {
//     Arena* prev_arena = *arena;
//     Arena* cur_arena = *arena;
//     bool head_insertion = false;
//     while (cur_arena != NULL && cur_arena->data != ptr) {
//         prev_arena = cur_arena;
//         cur_arena = cur_arena->next;
//     }
//     if (cur_arena == NULL)
//         return NULL;
//     if (prev_arena == cur_arena)
//         head_insertion = true;
//     if (cur_arena->size >= size) {
//         cur_arena->current = size;
//         return cur_arena->data;
//     }
//     size_t arena_size = size < BLOCK_SIZE ? BLOCK_SIZE : size;
//     Arena* new_arena = s_arena_create(arena_size);
//     if (new_arena == NULL)
//         return NULL;
//     memcpy(new_arena->data, cur_arena->data, cur_arena->current);
//     if (!head_insertion)
//         prev_arena->next = new_arena;
//     new_arena->next = cur_arena->next;
//     new_arena->current += size;
//     free(cur_arena->data);
//     free(cur_arena);
//     if (head_insertion)
//         *arena = new_arena;
//     return new_arena->data;
// }

// void arena_free(Arena* arena)
// {
//     while (arena != NULL) {
//         Arena* cur_arena = arena;
//         arena = arena->next;
//         free(cur_arena->data);
//         free(cur_arena);
//     }
// }

// void arena_free_block(Arena* arena, void* ptr)
// {
//     Arena* prev_arena = arena;
//     while (arena != NULL && arena->data != ptr) {
//         prev_arena = arena;
//         arena = arena->next;
//     }
//     if (arena == NULL)
//         return;
//     prev_arena->next = arena->next;
//     free(arena->data);
//     free(arena);
// }

void arena_print(Arena* arena)
{
    int arena_num = 0;
    while (arena != NULL) {
        arena_num++;
        printf("===== ARENA %d [size: %zu] =====\n", arena_num, arena->size);
        printf("--- Free list ---\n");
        Node* node = arena->free;
        while (node != NULL) {
            printf("%p [Size: %d, magic: %d]\n", node, node->header.size, node->header.magic);
            node = node->next;
        }
        printf("--- Allocated list ---\n");
        node = arena->allocated;
        while (node != NULL) {
            printf("%p [Size: %d, magic: %d]\n", node, node->header.size, node->header.magic);
            node = node->next;
        }
        arena = arena->next;
    }
}

void* arena_malloc_block(Arena* arena, size_t size) { return NULL; }

void* arena_free_block(Arena* arena, void* ptr) { }

// // assume it contains only one type of data, starting from the beginning of the data array
void* arena_realloc(Arena** arena, size_t size, void* ptr) { return NULL; }

void arena_free(Arena* arena) { }
