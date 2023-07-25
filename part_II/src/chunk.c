#include <assert.h>
#include <string.h>
#include "chunk.h"
#include "memory.h"
#include "vm.h"

void init_chunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->lines_capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    init_value_array(&chunk->constants);
}

// uses run-length encoding for line numbers
static void write_line(Chunk* chunk, int line)
{
    while (chunk->lines_capacity < line) {
        int old_capacity = chunk->lines_capacity;
        chunk->lines_capacity = GROW_CAPACITY(old_capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->lines_capacity);
        memset(
            &chunk->lines[old_capacity], 0, (chunk->lines_capacity - old_capacity) * sizeof(int));
    }
    chunk->lines[line - 1]++;
}

void write_chunk(Chunk* chunk, uint8_t byte, int line)
{
    assert(line > 0);
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->count++;
    write_line(chunk, line);
}

void free_chunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->lines_capacity);
    free_value_array(&chunk->constants);
    init_chunk(chunk);
}

int add_constant(Chunk* chunk, Value value)
{
    push(value);
    write_value_array(&chunk->constants, value);
    pop();
    return chunk->constants.count - 1;
}

int get_line(Chunk* chunk, int index)
{
    int line_count = 0;
    int tot_instructions = 0;
    while (index >= tot_instructions) {
        tot_instructions += chunk->lines[line_count];
        line_count++;
    }
    return line_count;
}

void write_constant(Chunk* chunk, Value value, int line)
{
    int constant_index = add_constant(chunk, value);
    write_chunk(chunk, OP_CONSTANT_LONG, line);
    // constant_index is 24 bit
    for (int opr = 0; opr < 3; opr++) {
        write_chunk(chunk, constant_index >> (8 * opr) & 0xFF, line);
    }
}