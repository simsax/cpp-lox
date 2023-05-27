#include <stdio.h>
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"

static VM vm;

static void reset_stack() { vm.stack_top = vm.stack; }

void init_VM() { reset_stack(); }

void free_VM() { }

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)                                                                              \
    do {                                                                                           \
        double b = pop();                                                                          \
        double a = pop();                                                                          \
        push(a op b);                                                                              \
    } while (false)

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));

#endif
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
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
#undef BINARY_OP
}

InterpretResult interpret(const char* source)
{
    compile(source);
    return INTERPRET_OK;
}

void push(Value value)
{
    *vm.stack_top = value;
    vm.stack_top++;
}

Value pop()
{
    vm.stack_top--;
    return *vm.stack_top;
}
