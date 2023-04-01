#pragma once
#include "LoxCallable.h"
#include "Stmt.h"

class LoxFunction : public LoxCallable {
public:
	LoxFunction(stmt::Function* declaration);
	std::any Call(Interpreter& interpreter,
		const std::vector<std::any>& arguments) override;
	size_t Arity() override;
	std::string ToString() override;

private:
	stmt::Function* m_Declaration;
};