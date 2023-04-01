#include "LoxFunction.h"
#include "Environment.h"
#include "Interpreter.h"

LoxFunction::LoxFunction(stmt::Function* declaration) :
	m_Declaration(declaration)
{
}

std::any LoxFunction::Call(Interpreter& interpreter,
	const std::vector<std::any>& arguments)
{
	Environment functionEnvironment = Environment(interpreter.m_Globals);
	for (uint32_t i = 0; i < arguments.size(); i++) {
		functionEnvironment.Define(m_Declaration->m_Params[i].lexeme, arguments[i]);
	}

	interpreter.ExecuteBlock(std::move(m_Declaration->m_Body), std::move(functionEnvironment));
	return nullptr;
}

size_t LoxFunction::Arity()
{
	return m_Declaration->m_Params.size();
}

std::string LoxFunction::ToString()
{
	return "<fn " + m_Declaration->m_Name.lexeme + ">";
}
