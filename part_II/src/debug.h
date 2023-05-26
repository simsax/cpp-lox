#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"
#include <time.h>

#define BENCHMARK_FUNC(function, ...)                                                              \
    do {                                                                                           \
        double start = (double)clock() / CLOCKS_PER_SEC;                                           \
        function(__VA_ARGS__);                                                                     \
        double end = (double)clock() / CLOCKS_PER_SEC;                                             \
        double elapsed_ms = (end - start) * 1000;                                                  \
        printf("[BENCHMARK] Took: %f ms\n", elapsed_ms);                                           \
    } while (false)

void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);

#endif