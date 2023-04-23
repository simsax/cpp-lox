#pragma once
#include "Environment.h"
#include "Expr.h"
#include "LoxCallable.h"


class LoxAnonFunction : public LoxCallable {
public:
    LoxAnonFunction(expr::AnonFunction* declaration, std::shared_ptr<Environment> closure);
    std::any Call(Interpreter& interpreter,
        const std::vector<std::any>& arguments) override;
    size_t Arity() const override;
    std::string ToString() const override;

private:
    expr::AnonFunction* m_Declaration;
    std::shared_ptr<Environment> m_Closure;
};