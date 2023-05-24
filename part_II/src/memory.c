#include "memory.h"
#include "arena.h"
#include <stdlib.h>

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

void free_arenas()
{
    arena_free(arenas.generic);
    arena_free(arenas.dynamic);
}

void* reallocate(void* pointer, size_t old_size, size_t new_size)
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
    if (result == NULL)
        exit(1);
    return result;
}
