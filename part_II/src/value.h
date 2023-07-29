#ifndef clox_value_h
#define clox_value_h

#include "common.h"
#include "string.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN ((uint64_t)0x7ffc000000000000)
#define TAG_NIL 1 // 01
#define TAG_FALSE 2 // 10
#define TAG_TRUE 3 // 11

typedef uint64_t Value;

#define IS_NUMBER(value) (((value & QNAN) != QNAN))
#define IS_NIL(value) ((value) == NIL_VAL)
#define IS_BOOL(value) (((value) | 1) == TRUE_VAL)
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define NUMBER_VAL(num) num_to_value(num)
#define NIL_VAL ((Value)(uint64_t)(QNAN | TAG_NIL))
#define TRUE_VAL ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define FALSE_VAL ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define BOOL_VAL(b) ((b) ? TRUE_VAL : FALSE_VAL)
#define OBJ_VAL(obj) (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

#define AS_NUMBER(value) value_to_num(value)
#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_OBJ(value) ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

static inline Value num_to_value(double num)
{
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

static inline double value_to_num(Value value)
{
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

#else

typedef enum { VAL_NIL, VAL_NUMBER, VAL_BOOL, VAL_OBJ } ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define NIL_VAL ((Value) { VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((Value) { VAL_NUMBER, { .number = value } })
#define BOOL_VAL(value) ((Value) { VAL_BOOL, { .boolean = value } })
#define OBJ_VAL(object) ((Value) { VAL_OBJ, { .obj = (Obj*)object } })

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#endif

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void init_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void free_value_array(ValueArray* array);
void print_value(Value value);
bool values_equal(Value a, Value b);

#endif