#include <stdexcept>
#include "Environment.h"
#include "Interpreter.h"

Environment::Environment() :
	m_Variables({})
{
}

void Environment::Define(const std::string& name, const std::any& value)
{
	m_Variables[name] = value;
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
