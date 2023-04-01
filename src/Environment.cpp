#include <stdexcept>
#include "Environment.h"
#include "Interpreter.h"

Environment::Environment() :
	m_Variables({}),
	m_Enclosing(nullptr)
{
}

Environment::Environment(std::shared_ptr<Environment> enclosing) :
	m_Variables({}),
	m_Enclosing(std::move(enclosing))
{
}

void Environment::Define(const std::string& name, const std::any& value)
{
	m_Variables[name] = value;
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
		return m_Variables.at(name.lexeme);
	}
	catch (const std::out_of_range&) {
		if (m_Enclosing != nullptr) {
			return m_Enclosing->Get(name);
		}
		throw RuntimeException("Undefined variable '" + name.lexeme + "'.", name);
	}
}
