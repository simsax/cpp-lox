#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    size_t stack_top;
    size_t stack_capacity;
    Value* stack;
} VM;

typedef enum { INTERPRET_OK, INTEPRET_COMPILE_ERROR, INTERPET_RUNTIME_ERROR } InterpretResult;

void init_VM();
void free_VM();
InterpretResult interpret(Chunk* chunk);
void push(Value value);
Value pop();

#endif