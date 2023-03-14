#include <stdexcept>
#include "Environment.h"
#include "Interpreter.h"

Environment::Environment() :
	m_Variables({})
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
	throw RuntimeException("Undefined variable '" + name.lexeme + "'.", name);
}

std::any Environment::Get(const Token& name) const
{
	try {
		return m_Variables.at(name.lexeme);
	}
	catch (const std::out_of_range&) {
		throw RuntimeException("Undefined variable '" + name.lexeme + "'.", name);
	}
}
