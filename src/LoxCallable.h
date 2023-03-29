#pragma once

#include <any>
#include <vector>
class Interpreter;

class LoxCallable {
public:
	virtual ~LoxCallable() = 0;
	virtual std::any Call(const Interpreter& interpreter,
		const std::vector<std::any>& arguments) = 0;
	virtual size_t Arity() = 0;
};

inline LoxCallable::~LoxCallable() = default;