#include "LoxInstance.h"
#include "Interpreter.h"
#include "LoxClass.h"
#include <memory>
#include <stdexcept>
#include <vector>

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> classPtr)
    : m_Class(std::move(classPtr))
{
}

std::any LoxInstance::Get(const Token& name, Interpreter& interpreter)
{
    if (m_Fields.contains(name.lexeme)) {
        return m_Fields.at(name.lexeme);
    }

    std::shared_ptr<LoxFunction> method = m_Class->FindMethod(name.lexeme);
    if (method != nullptr) {
        // it returns a shared ptr to a LoxCallable which contains a shared ptr to
        // this (LoxInstance)
        std::shared_ptr<LoxCallable> boundMethod = method->Bind(shared_from_this());
        if (method->IsGetter()) {
            return boundMethod->Call(interpreter, std::vector<std::any> {});
        }
        return boundMethod;
    }

    throw RuntimeException("Undefined property '" + name.lexeme + "'.", name);
}

void LoxInstance::Set(const Token& name, const std::any& value) { m_Fields[name.lexeme] = value; }

std::string LoxInstance::ToString() const { return "<" + m_Class->GetName() + " instance>"; }