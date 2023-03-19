#include <stdexcept>
#include "Environment.h"
#include "Interpreter.h"

Environment::Environment() :
	m_Enclosing(nullptr)
{
}

Environment::Environment(Environment* enclosing) :
	m_Enclosing(enclosing)
{
}

void Environment::Define(const Token& name, const std::any& value)
{
	m_Variables[name.lexeme] = value;
}

void Environment::Assign(const Token& name, const std::any& value)
{
	if (m_Variables.contains(name.lexeme)) {
		m_Variables[name.lexeme] = value;
		return;
	}
	if (m_Enclosing != nullptr) {
		m_Enclosing->Assign(name, value);
		return;
	}
	throw RuntimeException("Undefined variable '" + name.lexeme + "'.", name);
}

std::any Environment::Get(const Token& name) const
{
	try {
		std::any value = m_Variables.at(name.lexeme);
		if (value.type() == typeid(std::nullptr_t))
			throw RuntimeException("Variable '" + name.lexeme + "' has not been initialized.", name);
		return value;
	}
	catch (const std::out_of_range&) {
		if (m_Enclosing != nullptr) {
			return m_Enclosing->Get(name);
		}
		throw RuntimeException("Undefined variable '" + name.lexeme + "'.", name);
	}
}
