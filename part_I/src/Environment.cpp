#include <stdexcept>
#include <stdint.h>
#include "Environment.h"
#include "Interpreter.h"
#include <memory>
#include <stdexcept>

Environment::Environment() :
	m_Enclosing(nullptr)
{
}

Environment::Environment(std::shared_ptr<Environment> enclosing) :
	m_Enclosing(std::move(enclosing))
{
}

void Environment::Define(const std::string& name, const std::any& value)
{
	m_Globals[name] = value;
}

void Environment::DefineLocal(const std::any& value) {
	m_Locals.emplace_back(value);
}

void Environment::Assign(const Token& name, const std::any& value)
{
	if (m_Globals.contains(name.lexeme)) {
		m_Globals[name.lexeme] = value;
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
		std::any value = m_Globals.at(name.lexeme);
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

std::any Environment::GetAt(uint32_t distance, uint32_t variableIndex)
{
	return Ancestor(distance)->m_Locals[variableIndex];
}

void Environment::AssignAt(uint32_t distance, uint32_t variableIndex, const std::any& value)
{
	Ancestor(distance)->m_Locals[variableIndex] = value;
}

Environment* Environment::Ancestor(int distance)
{
    Environment* environment = this;
    for (int i = 0; i < distance; i++) {
        environment = environment->m_Enclosing.get();
    }
    return environment;
}

std::shared_ptr<Environment> Environment::GetEnclosing() { return m_Enclosing; }