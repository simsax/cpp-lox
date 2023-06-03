#include "memory.h"
#include "arena.h"
#include "vm.h"
#include "object.h"
#include <stdlib.h>
#include "stdio.h"
#include <assert.h>

typedef struct {
    // used for small memory objects that don't grow
    Arena* generic;

    // used for dynamic arrays
    Arena* dynamic;
} Arenas;

static Arenas arenas;

void init_arenas()
{
    arenas.generic = arena_create();
    arenas.dynamic = arena_create();
}

void free_dynamic_arena() { arena_free(arenas.dynamic); }

void free_generic_arena() { arena_free(arenas.generic); }

void* reallocate(void* pointer, size_t old_size, size_t new_size)
{
    // TODO: implement realloc
    if (new_size == 0) {
        // TODO: generic heap implementation,
        // header before each block to track the size, free adds the block to a free
        // list, allocation walks the free list to find a big enough block and so on
        return NULL;
    }

    void* block = arena_malloc(arenas.generic, new_size);
    if (block == NULL) {
        fprintf(stderr, "Not enough memory to allocate block");
        exit(74);
    }
    return block;
}

void* reallocate_dynamic(void* pointer, size_t old_size, size_t new_size)
{
    if (new_size == 0) {
        arena_free_block(arenas.dynamic, pointer);
        return NULL;
    }

    if (pointer == NULL) {
        if (arenas.dynamic->current == 0)
            return arena_malloc(arenas.dynamic, new_size);
        return arena_malloc_block(arenas.dynamic, new_size);
    }

    void* result = arena_realloc(&arenas.dynamic, new_size, pointer);
    if (result == NULL) {
        fprintf(stderr, "Not enough memory to allocate block");
        exit(74);
    }
    return result;
}
