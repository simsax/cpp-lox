#include "LoxClass.h"
#include "LoxInstance.h"
#include <memory>

LoxClass::LoxClass(const std::string& name)
	: m_Name(name)
{
}

std::any LoxClass::Call(Interpreter&, const std::vector<std::any>&)
{
	return std::make_shared<LoxInstance>(*this);
}

size_t LoxClass::Arity() const
{
	return 0;
}

std::string LoxClass::ToString() const
{
	return "<class " + m_Name + ">";
}

std::string LoxClass::GetName() const
{
	return m_Name;
}