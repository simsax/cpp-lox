#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    size_t stack_top;
    size_t stack_capacity;
    Value* stack;
    Table strings;
    Obj* objects;
} VM;

typedef enum { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR } InterpretResult;

extern VM vm;

void init_VM();
void free_VM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif