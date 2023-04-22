#pragma once

#include "LoxCallable.h"
#include <string>

class LoxClass : public LoxCallable {
public:
    LoxClass(const std::string& name);

    std::any Call(Interpreter& interpreter,
        const std::vector<std::any>& arguments) override;
    size_t Arity() const override;
    std::string ToString() const override;
    std::string GetName() const;

private:
    std::string m_Name;
};