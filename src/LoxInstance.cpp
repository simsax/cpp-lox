#include "LoxInstance.h"
#include "Interpreter.h"
#include <stdexcept>

LoxInstance::LoxInstance(const LoxClass& classRef)
	: m_Class(classRef)
{
}

std::any LoxInstance::Get(const Token& name) const
{
	if (m_Fields.contains(name.lexeme)) {
		return m_Fields.at(name.lexeme);
	}

	std::shared_ptr<LoxCallable> method = m_Class.FindMethod(name.lexeme);
	if (method != nullptr) {
		return method;
	}

	throw RuntimeException("Undefined property '" + name.lexeme + "'.", name);
}

void LoxInstance::Set(const Token& name, const std::any& value)
{
	m_Fields[name.lexeme] = value;
}

std::string LoxInstance::ToString() const
{
	return "<" + m_Class.GetName() + " instance>";
}