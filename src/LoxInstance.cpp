#include "LoxInstance.h"
#include "Interpreter.h"
#include <stdexcept>

LoxInstance::LoxInstance(const LoxClass& classRef)
    : m_Class(classRef)
{
}

std::any LoxInstance::Get(const Token& name) const
{
    try {
        return m_Fields.at(name.lexeme);
    }

    catch (const std::out_of_range&) {
        throw RuntimeException("Undefined property '" + name.lexeme + "'.", name);
    }
}

void LoxInstance::Set(const Token& name, const std::any& value)
{
    m_Fields[name.lexeme] = value;
}

std::string LoxInstance::ToString() const
{
    return "<" + m_Class.GetName() + " instance>";
}