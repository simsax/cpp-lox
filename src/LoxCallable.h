#pragma once

#include <any>
#include <vector>
#include <string>
class Interpreter;

class LoxCallable {
public:
	virtual ~LoxCallable() = 0;
	virtual std::any Call(Interpreter& interpreter,
		const std::vector<std::any>& arguments) = 0;
	virtual size_t Arity() = 0;
	virtual std::string ToString() = 0;
};

inline LoxCallable::~LoxCallable() = default;