#include "vm.h"
#include "common.h"
#include <stdio.h>
#include "debug.h"

static VM vm;

void init_VM() { }

void free_VM() { }

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#ifdef DEBUG_TRACE_EXECUTION

    disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));

#endif

    for (;;) {
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            print_value(constant);
            printf("\n");
            break;
        }
        case OP_RETURN: {
            return INTERPRET_OK;
        }
        default:
            break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}