#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value* stack_top;
    Value stack[STACK_MAX];
    Table strings;
    Table globals;
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