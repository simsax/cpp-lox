#pragma once
#include "LoxCallable.h"
#include "Environment.h"
#include "Expr.h"

class LoxAnonFunction : public LoxCallable {
public:
	LoxAnonFunction(expr::AnonFunction* declaration, std::shared_ptr<Environment> closure);
	std::any Call(Interpreter& interpreter,
		const std::vector<std::any>& arguments) override;
	size_t Arity() override;
	std::string ToString() override;

private:
	expr::AnonFunction* m_Declaration;
	std::shared_ptr<Environment> m_Closure;
};