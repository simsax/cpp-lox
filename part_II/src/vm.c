#include <stdio.h>
#include "vm.h"
#include "common.h"
#include "debug.h"
#include "memory.h"

static VM vm;

static void reset_stack() { vm.stack_top = 0; }

void init_VM()
{
    reset_stack();
    vm.stack_capacity = 0;
    vm.stack = NULL;
}

void free_VM() { }

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define PUSH_CONSTANT_LONG()                                                                       \
    do {                                                                                           \
        int constant_index = 0;                                                                    \
        for (int opr = 0; opr < 3; opr++) {                                                        \
            constant_index |= READ_BYTE() << (8 * opr);                                            \
        }                                                                                          \
        push(vm.chunk->constants.values[constant_index]);                                          \
    } while (false)
#define BINARY_OP(op)                                                                              \
    do {                                                                                           \
        double b = pop();                                                                          \
        double a = pop();                                                                          \
        push(a op b);                                                                              \
    } while (false)

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (size_t slot = 0; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_value(vm.stack[slot]);
            printf(" ]");
        }
        printf("\n");
        disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));

#endif
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
        case OP_CONSTANT_LONG: {
            PUSH_CONSTANT_LONG();
            break;
        }
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_NEGATE:
            push(-pop());
            break;
        case OP_ADD:
            BINARY_OP(+);
            break;
        case OP_SUBTRACT:
            BINARY_OP(-);
            break;
        case OP_MULTIPLY:
            BINARY_OP(*);
            break;
        case OP_DIVIDE:
            BINARY_OP(/);
            break;
        case OP_RETURN: {
            print_value(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        default:
            break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef PUSH_CONSTANT_LONG
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}

void push(Value value)
{
    // now push operation is slower due to the additional checks required for a dynamic stack;
    // cannot use a pointer to top of stack because the stack might be copied to another
    // location on the heap in case realloc can't find enough contiguous space, so using an index
    // is required
    // TODO: fix this
    if (vm.stack_capacity <= vm.stack_top) {
        size_t old_capacity = vm.stack_capacity;
        vm.stack_capacity = GROW_CAPACITY(old_capacity);
        vm.stack = GROW_ARRAY(Value, vm.stack, old_capacity, vm.stack_capacity);
    }
    vm.stack[vm.stack_top] = value;
    vm.stack_top++;
}

Value pop()
{
    vm.stack_top--;
    return vm.stack[vm.stack_top];
}
