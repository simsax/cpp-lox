#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "vm.h"
#include "memory.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"

VM vm;

static void reset_stack()
{
    vm.stack_top = 0;
    vm.frame_count = 0;
    vm.open_upvalues = NULL;
}

static void runtime_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frame_count - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - frame->closure->function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    reset_stack();
}

static bool clock_native(int arg_count, Value* args, Value* result)
{
    *result = NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
    return true;
}

static bool sqrt_native(int arg_count, Value* args, Value* result)
{
    Value input = args[0];
    if (!IS_NUMBER(input)) {
        runtime_error("Input to sqrt function must be a number.");
        return false;
    }
    double val = AS_NUMBER(input);
    *result = NUMBER_VAL(sqrt(val));
    return true;
}

static bool input_native(int arg_count, Value* args, Value* result)
{
    Value prompt = args[0];
    if (!IS_STRING(prompt)) {
        runtime_error("Input prompt must be a string.");
        return false;
    }
    printf("%s", AS_CSTRING(prompt));
    char c;
    size_t length = 0;
    size_t capacity = 256;
    char* string = ALLOCATE(char, capacity);
    while ((c = getchar()) != '\n') {
        if (length + 1 >= capacity) {
            size_t old_capacity = capacity;
            capacity = GROW_CAPACITY(capacity);
            string = GROW_ARRAY(char, string, old_capacity, capacity);
        }
        string[length++] = c;
    }
    string[length++] = '\0';
    ObjString* obj_string = take_string(string, length);
    *result = OBJ_VAL(obj_string);
    return true;
}

static void define_native(const char* name, NativeFn function, int arity)
{
    push(OBJ_VAL(copy_string(name, (int)strlen(name))));
    push(OBJ_VAL(new_native(function, arity)));
    table_set(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

// returns a value from the stack but doesn't pop it
static Value peek(int distance) { return vm.stack[vm.stack_top - 1 - distance]; }

static bool is_falsey(Value value) { return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value)); }

static void concatenate()
{
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';
    ObjString* result = take_string(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

void init_VM()
{
    reset_stack();
    vm.stack_capacity = 0;
    vm.stack = NULL;
    vm.objects = NULL;
    vm.bytes_allocated = 0;
    vm.next_gc = 1024 * 1024;
    vm.gray_count = 0;
    vm.gray_capacity = 0;
    vm.gray_stack = NULL;
    init_table(&vm.strings);
    init_table(&vm.globals);

    define_native("clock", clock_native, 0);
    define_native("sqrt", sqrt_native, 1);
    define_native("input", input_native, 1);
}

void free_VM()
{
    free_table(&vm.globals);
    free_table(&vm.strings);
    free_objects();
}

static bool call(ObjClosure* closure, int arg_count)
{
    if (arg_count != closure->function->arity) {
        runtime_error("Expected %d arguments but got %d.", closure->function->arity, arg_count);
        return false;
    }
    if (vm.frame_count == FRAMES_MAX) {
        runtime_error("Stack overflow.");
        return false;
    }
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stack_top - arg_count - 1;
    return true;
}

static bool call_native(ObjNative* native, int arg_count)
{
    if (arg_count != native->arity) {
        runtime_error("Expected %d arguments but got %d.", native->arity, arg_count);
        return false;
    }
    NativeFn native_function = native->function;
    Value result = NIL_VAL;
    bool success = native_function(arg_count, &vm.stack[vm.stack_top - arg_count], &result);
    vm.stack_top -= arg_count + 1;
    push(result);
    return success;
}

static bool call_value(Value callee, int arg_count)
{
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
        case OBJ_NATIVE:
            return call_native(AS_NATIVE(callee), arg_count);
        case OBJ_CLOSURE:
            return call(AS_CLOSURE(callee), arg_count);
        default:
            break;
        }
    }
    runtime_error("Can only call functions and classes.");
    return false;
}

static ObjUpvalue* capture_upvalue(Value* local)
{
    ObjUpvalue* prev_upvalue = NULL;
    ObjUpvalue* upvalue = vm.open_upvalues;

    while (upvalue != NULL && upvalue->location > local) {
        prev_upvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* created_upvalue = new_upvalue(local);
    created_upvalue->next = upvalue;
    if (prev_upvalue == NULL) {
        vm.open_upvalues = created_upvalue;
    } else {
        prev_upvalue->next = created_upvalue;
    }
    return created_upvalue;
}

static void close_upvalues(Value* last)
{
    while (vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
        ObjUpvalue* upvalue = vm.open_upvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.open_upvalues = upvalue->next;
    }
}

static InterpretResult run()
{
    CallFrame* frame = &vm.frames[vm.frame_count - 1];
    register uint8_t* ip = frame->ip;
#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define PUSH_CONSTANT_LONG()                                                                       \
    do {                                                                                           \
        int constant_index = 0;                                                                    \
        for (int opr = 0; opr < 3; opr++) {                                                        \
            constant_index |= READ_BYTE() << (8 * opr);                                            \
        }                                                                                          \
        push(frame->closure->function->chunk.constants.values[constant_index]);                    \
    } while (false)
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define BINARY_OP(value_type, op)                                                                  \
    do {                                                                                           \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                                          \
            runtime_error("Operands must be numbers.");                                            \
            return INTERPRET_RUNTIME_ERROR;                                                        \
        }                                                                                          \
        double b = AS_NUMBER(pop());                                                               \
        TOP_STACK_VAL = value_type(AS_NUMBER(TOP_STACK_VAL) op b);                                 \
    } while (false)
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define TOP_STACK_VAL vm.stack[vm.stack_top - 1]

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (size_t slot = 0; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_value(vm.stack[slot]);
            printf(" ]");
        }
        printf("\n");
        disassemble_instruction(
            &frame->closure->function->chunk, (int)(ip - frame->closure->function->chunk.code));

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
        case OP_ADD: {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                concatenate();
            } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            } else {
                runtime_error("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
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
            Value result = pop();
            close_upvalues(&vm.stack[frame->slots]);
            vm.frame_count--;
            if (vm.frame_count == 0) {
                pop();
                return INTERPRET_OK;
            }

            vm.stack_top = frame->slots;
            push(result);
            frame = &vm.frames[vm.frame_count - 1];
            ip = frame->ip;
            break;
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
        case OP_PRINT: {
            print_value(pop());
            printf("\n");
            break;
        }
        case OP_POP:
            pop();
            break;
        case OP_DEFINE_GLOBAL: {
            ObjString* name = READ_STRING();
            table_set(&vm.globals, name, peek(0));
            pop();
            break;
        }
        case OP_GET_GLOBAL: {
            ObjString* name = READ_STRING();
            Value value;
            if (!table_get(&vm.globals, name, &value)) {
                runtime_error("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            break;
        }
        case OP_SET_GLOBAL: {
            ObjString* name = READ_STRING();
            Value value;
            if (table_set(&vm.globals, name, peek(0))) {
                table_delete(&vm.globals, name);
                runtime_error("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            // setting a variable doesn't pop the value off the stack because
            // assignment is an expression
            break;
        }
        case OP_GET_LOCAL: {
            uint8_t slot = READ_BYTE();
            push(vm.stack[frame->slots + slot]);
            break;
        }
        case OP_SET_LOCAL: {
            uint8_t slot = READ_BYTE();
            vm.stack[frame->slots + slot] = peek(0);
            break;
        }
        case OP_JUMP_IF_FALSE: {
            uint16_t offset = READ_SHORT();
            if (is_falsey(peek(0)))
                ip += offset;
            break;
        }
        case OP_JUMP: {
            uint16_t offset = READ_SHORT();
            ip += offset;
            break;
        }
        case OP_LOOP: {
            uint16_t offset = READ_SHORT();
            ip -= offset;
            break;
        }
        case OP_CALL: {
            uint8_t arg_count = READ_BYTE();
            frame->ip = ip;
            if (!call_value(peek(arg_count), arg_count)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frame_count - 1];
            ip = frame->ip;
            break;
        }
        case OP_JUMP_IF_NOT_EQUAL: {
            uint16_t offset = READ_SHORT();
            if (!values_equal(peek(0), peek(1)))
                ip += offset;
            break;
        }
        case OP_CLOSURE: {
            ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
            ObjClosure* closure = new_closure(function);
            push(OBJ_VAL(closure));
            for (int i = 0; i < closure->upvalue_count; i++) {
                uint8_t is_local = READ_BYTE();
                uint8_t index = READ_BYTE();
                if (is_local) {
                    closure->upvalues[i] = capture_upvalue(&vm.stack[frame->slots + index]);
                } else {
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }
            break;
        }
        case OP_GET_UPVALUE: {
            uint8_t slot = READ_BYTE();
            push(*frame->closure->upvalues[slot]->location);
            break;
        }
        case OP_SET_UPVALUE: {
            uint8_t slot = READ_BYTE();
            *frame->closure->upvalues[slot]->location = peek(0);
            break;
        }
        case OP_CLOSE_UPVALUE: {
            close_upvalues(&vm.stack[vm.stack_top - 1]);
            pop();
            break;
        }
        default:
            break;
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef PUSH_CONSTANT_LONG
#undef BINARY_OP
#undef TOP_STACK_VAL
#undef READ_STRING
}

InterpretResult interpret(const char* source)
{
    ObjFunction* function = compile(source);
    if (function == NULL)
        return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = new_closure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);

    return run();
}

void push(Value value)
{
    // now push operation is slower due to the additional checks required for a dynamic stack;
    // cannot use a pointer to top of stack because the stack might be copied to another
    // location on the heap in case realloc can't find enough contiguous space, so using an index
    // is required
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
