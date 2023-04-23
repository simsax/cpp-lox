#pragma once

#include <any>
#include <string>
#include <vector>

class Interpreter;

class LoxCallable {
public:
    virtual ~LoxCallable() = 0;
    virtual std::any Call(Interpreter& interpreter,
        const std::vector<std::any>& arguments)
        = 0;
    virtual size_t Arity() const = 0;
    virtual std::string ToString() const = 0;
};

inline LoxCallable::~LoxCallable() = default;