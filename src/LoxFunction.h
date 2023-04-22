#pragma once
#include "Environment.h"
#include "LoxCallable.h"
#include "Stmt.h"
#include <memory>

class LoxInstance;

class LoxFunction : public LoxCallable {
public:
    LoxFunction(stmt::Function* declaration, std::shared_ptr<Environment> closure);
    std::any Call(Interpreter& interpreter,
        const std::vector<std::any>& arguments) override;
    size_t Arity() const override;
    std::string ToString() const override;
    std::shared_ptr<LoxCallable> Bind(std::shared_ptr<LoxInstance> instance);

private:
    stmt::Function* m_Declaration;
    std::shared_ptr<Environment> m_Closure;
};