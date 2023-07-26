#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)
#define IS_CLOSURE(value) is_obj_type(value, OBJ_CLOSURE)
#define IS_CLASS(value) is_obj_type(value, OBJ_CLASS)
#define IS_INSTANCE(value) is_obj_type(value, OBJ_INSTANCE)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE
} ObjType;

struct Obj {
    ObjType type;
    bool is_marked;
    struct Obj* next;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

typedef struct {
    Obj obj;
    int arity;
    int upvalue_count;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
};

typedef struct ObjUpvalue ObjUpvalue;

typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalue_count;
} ObjClosure;

typedef struct {
    Obj obj;
    ObjString* name;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;

typedef Value (*NativeFn)(int arg_count, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

static inline bool is_obj_type(Value value, ObjType obj_type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == obj_type;
}

ObjString* copy_string(const char* chars, int length);
ObjString* take_string(char* chars, int length);
ObjFunction* new_function();
ObjNative* new_native(NativeFn function);
ObjClosure* new_closure(ObjFunction* function);
ObjUpvalue* new_upvalue(Value* slot);
ObjClass* new_class(ObjString* name);
ObjInstance* new_instance(ObjClass* klass);
void print_object(Value value);

#endif