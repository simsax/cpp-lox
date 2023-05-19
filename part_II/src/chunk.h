#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum { OP_RETURN, OP_CONSTANT, OP_CONSTANT_LONG } OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int lines_capacity;
    int* lines;
    ValueArray constants;
} Chunk;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte, int line);
void free_chunk(Chunk* chunk);

// adds a value to chunk's constants array and returns its index
int add_constant(Chunk* chunk, Value value);

// given the index of an instruction, return the line where it occurs
int get_line(Chunk* chunk, int index);

// adds value to chunk’s constant array and then writes
// an appropriate instruction to load the constant
void write_constant(Chunk* chunk, Value value, int line);

#endif