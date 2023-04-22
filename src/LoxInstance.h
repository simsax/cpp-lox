#pragma once

#include "LoxClass.h"
#include "Token.h"
#include <unordered_map>

class LoxInstance {
public:
    LoxInstance(const LoxClass& classRef);
    std::any Get(const Token& name) const;
    void Set(const Token& name, const std::any& value);
    std::string ToString() const;

private:
    const LoxClass& m_Class;
    std::unordered_map<std::string, std::any> m_Fields;
};