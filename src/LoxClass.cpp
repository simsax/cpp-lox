#include "LoxClass.h"
#include "LoxInstance.h"
#include <memory>

LoxClass::LoxClass(const std::string& name, MethodMap methods)
    : m_Name(name)
    , m_Methods(std::move(methods))
{
}

std::any LoxClass::Call(Interpreter& interpreter, const std::vector<std::any>& arguments)
{
    std::shared_ptr<LoxInstance> instance = std::make_shared<LoxInstance>(*this);
    std::shared_ptr<LoxFunction> initializer = FindMethod("init");
    if (initializer != nullptr) {
        initializer->Bind(instance)->Call(interpreter, arguments);
    }

    return instance;
}

size_t LoxClass::Arity() const
{
    std::shared_ptr<LoxFunction> initializer = FindMethod("init");
    if (initializer == nullptr) {
        return 0;
    }
    return initializer->Arity();
}

std::string LoxClass::ToString() const
{
    return "<class " + m_Name + ">";
}

std::string LoxClass::GetName() const
{
    return m_Name;
}

std::shared_ptr<LoxFunction> LoxClass::FindMethod(const std::string& name) const
{
    if (m_Methods.contains(name)) {
        return m_Methods.at(name);
    }
    return nullptr;
}