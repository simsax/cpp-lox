#include "LoxInstance.h"
#include "Interpreter.h"
#include <stdexcept>

LoxInstance::LoxInstance(const LoxClass& classRef)
    : m_Class(classRef)
{
}

std::any LoxInstance::Get(const Token& name)
{
    if (m_Fields.contains(name.lexeme)) {
        return m_Fields.at(name.lexeme);
    }

    std::shared_ptr<LoxFunction> method = m_Class.FindMethod(name.lexeme);
    if (method != nullptr) {
        // LoxInstance returns a shared ptr to a LoxFunction which contains a shared ptr to
        // this (LoxInstance)
        return method->Bind(shared_from_this());
    }

    throw RuntimeException("Undefined property '" + name.lexeme + "'.", name);
}

void LoxInstance::Set(const Token& name, const std::any& value) { m_Fields[name.lexeme] = value; }

std::string LoxInstance::ToString() const { return "<" + m_Class.GetName() + " instance>"; }