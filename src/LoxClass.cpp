#include "LoxClass.h"
#include "LoxInstance.h"
#include <memory>

LoxClass::LoxClass(const std::string& name, MethodMap methods)
    : m_Name(name)
    , m_Methods(std::move(methods))
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

std::shared_ptr<LoxCallable> LoxClass::FindMethod(const std::string& name) const
{
    if (m_Methods.contains(name)) {
        return m_Methods.at(name);
    }
    return nullptr;
}