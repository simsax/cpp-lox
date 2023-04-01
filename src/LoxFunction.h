#pragma once
#include "LoxCallable.h"
#include "Environment.h"
#include "Stmt.h"

class LoxFunction : public LoxCallable {
public:
	LoxFunction(stmt::Function* declaration, std::shared_ptr<Environment> closure);
	std::any Call(Interpreter& interpreter,
		const std::vector<std::any>& arguments) override;
	size_t Arity() override;
	std::string ToString() override;

private:
	stmt::Function* m_Declaration;
	std::shared_ptr<Environment> m_Closure;
};