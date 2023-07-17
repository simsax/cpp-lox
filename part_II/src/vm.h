#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjFunction* function;
    uint8_t* ip;
    size_t first_slot; // index to the VM's value stack at the first slot that this function can use
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frame_count;
    size_t stack_top;
    size_t stack_capacity;
    Value* stack;
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