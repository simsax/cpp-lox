#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "vm.h"
#include "value.h"
#include "table.h"

#define ALLOCATE_OBJ(type, object_type) (type*)allocate_object(sizeof(type), object_type)

static Obj* allocate_object(size_t size, ObjType type)
{
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->next = vm.objects;
    vm.objects = object;
    return object;
}

static ObjString* allocate_string(const char* chars, int length, uint32_t hash)
{
    // flexible array member
    ObjString* string
        = (ObjString*)allocate_object(sizeof(ObjString) + (length + 1) * sizeof(char), OBJ_STRING);
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';
    string->length = length;
    string->hash = hash;
    // hash set -> we only care about the keys
    table_set(&vm.strings, string, NIL_VAL);
    return string;
}

static uint32_t hash_string(const char* key, int length)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* copy_string(const char* chars, int length)
{
    uint32_t hash = hash_string(chars, length);
    ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
    if (interned != NULL)
        return interned;
    return allocate_string(chars, length, hash);
}

ObjString* take_string(char* chars, int length)
{
    ObjString* string = copy_string(chars, length);
    FREE(char, chars);
    return string;
}

static void print_function(ObjFunction* function)
{
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void print_object(Value value)
{
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    case OBJ_FUNCTION:
        print_function(AS_FUNCTION(value));
        break;
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
    default:
        break;
    }
}

ObjString* make_string(const char* chars) { return copy_string(chars, strlen(chars)); }

ObjFunction* new_function()
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    init_chunk(&function->chunk);
    return function;
}

ObjNative* new_native(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}
