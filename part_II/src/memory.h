#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
#include "value.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)
#define GROW_ARRAY(type, pointer, old_count, new_count)                                            \
    (type*)reallocate(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))
#define FREE_ARRAY(type, pointer, old_count)                                                       \
    (type*)reallocate(pointer, sizeof(type) * (old_count), 0)
#define ALLOCATE(type, count) (type*)reallocate(NULL, 0, count * sizeof(type))
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

void* reallocate(void* pointer, size_t old_size, size_t new_size);
void mark_object(Obj* object);
void mark_value(Value value);
void free_objects();
void collect_garbage();

#endif