#include "LoxAnonFunction.h"
#include "Interpreter.h"

LoxAnonFunction::LoxAnonFunction(expr::AnonFunction* declaration, std::shared_ptr<Environment> closure) :
	m_Declaration(declaration), m_Closure(std::move(closure))
{
}

std::any LoxAnonFunction::Call(Interpreter& interpreter,
	const std::vector<std::any>& arguments)
{
	std::shared_ptr<Environment> functionEnvironment = std::make_shared<Environment>(m_Closure);
	for (uint32_t i = 0; i < arguments.size(); i++) {
		functionEnvironment->Define(m_Declaration->m_Params[i].lexeme, arguments[i]);
	}

	try {
		interpreter.ExecuteBlock(std::move(m_Declaration->m_Body), functionEnvironment);
	}
	catch (const Return& returnValue) {
		return returnValue.GetValue();
	}
	return nullptr;
}

size_t LoxAnonFunction::Arity() const
{
	return m_Declaration->m_Params.size();
}

std::string LoxAnonFunction::ToString() const
{
	return "<anonymous fn>";
}
