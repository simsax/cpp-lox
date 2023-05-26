#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "arena.h"
#include "memory.h"
#include "vm.h"
#include <stdio.h>
#include <assert.h>

int main(int argc, const char* argv[])
{
    init_arenas();
    init_VM();
    Chunk chunk;
    init_chunk(&chunk);

    for (int i = 0; i < 1000000; i++) {
        write_constant(&chunk, 5.6, 123);
        write_chunk(&chunk, OP_NEGATE, 123);
    }

    write_constant(&chunk, 5, 123);
    write_constant(&chunk, 5, 123);
    write_chunk(&chunk, OP_MULTIPLY, 123);

    write_chunk(&chunk, OP_RETURN, 123);
    BENCHMARK_FUNC(interpret, &chunk);
    free_VM();
    free_arenas();
    return 0;
}