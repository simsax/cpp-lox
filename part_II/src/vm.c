#include <stdio.h>
#include <stdarg.h>
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"

static VM vm;

static void reset_stack() { vm.stack_top = 0; }

static void runtime_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction_index = vm.ip - vm.chunk->code - 1;
    int line = get_line(vm.chunk, instruction_index);
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

// returns a value from the stack but doesn't pop it
static Value peek(int distance) { return vm.stack[vm.stack_top - 1 - distance]; }

static bool is_falsey(Value value) { return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value)); }

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
#define TOP_STACK_VAL vm.stack[vm.stack_top - 1]
#define PUSH_CONSTANT_LONG()                                                                       \
    do {                                                                                           \
        int constant_index = 0;                                                                    \
        for (int opr = 0; opr < 3; opr++) {                                                        \
            constant_index |= READ_BYTE() << (8 * opr);                                            \
        }                                                                                          \
        push(vm.chunk->constants.values[constant_index]);                                          \
    } while (false)
#define BINARY_OP(value_type, op)                                                                  \
    do {                                                                                           \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                                          \
            runtime_error("Operands must be numbers.");                                            \
            return INTERPRET_RUNTIME_ERROR;                                                        \
        }                                                                                          \
        double b = AS_NUMBER(pop());                                                               \
        TOP_STACK_VAL = value_type(AS_NUMBER(TOP_STACK_VAL) op b);                                 \
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
            if (!IS_NUMBER(peek(0))) {
                runtime_error("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            TOP_STACK_VAL = NUMBER_VAL(-AS_NUMBER(TOP_STACK_VAL));
            break;
        case OP_ADD:
            BINARY_OP(NUMBER_VAL, +);
            break;
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;
        case OP_RETURN: {
            print_value(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        case OP_NIL:
            push(NIL_VAL);
            break;
        case OP_FALSE:
            push(BOOL_VAL(false));
            break;
        case OP_TRUE:
            push(BOOL_VAL(true));
            break;
        case OP_NOT:
            TOP_STACK_VAL = BOOL_VAL(is_falsey(TOP_STACK_VAL));
            break;
        case OP_EQUAL: {
            Value b = pop();
            TOP_STACK_VAL = BOOL_VAL(values_equal(TOP_STACK_VAL, b));
            break;
        }
        case OP_NOT_EQUAL: {
            Value b = pop();
            TOP_STACK_VAL = BOOL_VAL(!values_equal(TOP_STACK_VAL, b));
            break;
        }
        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;
        case OP_GREATER_EQUAL:
            BINARY_OP(BOOL_VAL, >=);
            break;
        case OP_LESS_EQUAL:
            BINARY_OP(BOOL_VAL, <=);
            break;
        default:
            break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef PUSH_CONSTANT_LONG
#undef BINARY_OP
#undef TOP_STACK_VAL
}

InterpretResult interpret(const char* source)
{
    Chunk chunk;
    init_chunk(&chunk);

    if (!compile(source, &chunk)) {
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    return result;
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
