#pragma once

#include "LoxClass.h"
#include "Token.h"
#include <memory>
#include <unordered_map>

class LoxInstance : public std::enable_shared_from_this<LoxInstance> {
public:
    LoxInstance(const LoxClass& classRef);
    std::any Get(const Token& name);
    void Set(const Token& name, const std::any& value);
    std::string ToString() const;

private:
    const LoxClass& m_Class;
    std::unordered_map<std::string, std::any> m_Fields;
};