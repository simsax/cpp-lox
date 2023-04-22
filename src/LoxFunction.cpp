#include "LoxFunction.h"
#include "Interpreter.h"

LoxFunction::LoxFunction(stmt::Function* declaration, std::shared_ptr<Environment> closure)
    : m_Declaration(declaration)
    , m_Closure(std::move(closure))
{
}

std::any LoxFunction::Call(Interpreter& interpreter, const std::vector<std::any>& arguments)
{
    std::shared_ptr<Environment> functionEnvironment = std::make_shared<Environment>(m_Closure);
    for (uint32_t i = 0; i < arguments.size(); i++) {
        functionEnvironment->Define(m_Declaration->m_Params[i].lexeme, arguments[i]);
    }

    try {
        interpreter.ExecuteBlock(std::move(m_Declaration->m_Body), functionEnvironment);
    } catch (const Return& returnValue) {
        return returnValue.GetValue();
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
