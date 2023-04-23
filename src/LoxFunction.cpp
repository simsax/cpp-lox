#include "LoxFunction.h"
#include "Interpreter.h"

LoxFunction::LoxFunction(stmt::Function* declaration, std::shared_ptr<Environment> closure,
    bool isInitializer)
    : m_Declaration(declaration)
    , m_Closure(std::move(closure))
    , m_IsInitializer(isInitializer)
{
}

std::any LoxFunction::Call(Interpreter& interpreter, const std::vector<std::any>& arguments)
{
    std::shared_ptr<Environment> functionEnvironment = std::make_shared<Environment>(m_Closure);
    for (uint32_t i = 0; i < arguments.size(); i++) {
        functionEnvironment->DefineLocal(arguments[i]);
    }

    try {
        interpreter.ExecuteBlock(std::move(m_Declaration->m_Body), functionEnvironment);
    } catch (const Return& returnValue) {
        if (m_IsInitializer)
            return m_Closure->GetAt(0, 0);
        return returnValue.GetValue();
    }

    if (m_IsInitializer) {
        return m_Closure->GetAt(0, 0);
    }
    return nullptr;
}

size_t LoxFunction::Arity() const
{
    return m_Declaration->m_Params.size();
}

std::string LoxFunction::ToString() const
{
    return "<fn " + m_Declaration->m_Name.lexeme + ">";
}

std::shared_ptr<LoxCallable> LoxFunction::Bind(std::shared_ptr<LoxInstance> instance)
{
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(m_Closure);
    environment->DefineLocal(instance);
    return std::make_shared<LoxFunction>(m_Declaration, environment, m_IsInitializer);
}