#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum { OBJ_STRING } ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
};

struct ObjString {
    Obj obj;
    int length;
    uint32_t hash;
    char chars[];
};

static inline bool is_obj_type(Value value, ObjType obj_type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == obj_type;
}

ObjString* copy_string(const char* chars, int length);
ObjString* take_string(char* chars, int length);
void print_object(Value value);

#endif