#pragma once
#include "LoxCallable.h"
#include "Token.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>


class Environment {
public:
    Environment();
    Environment(std::shared_ptr<Environment> enclosing);
    void Define(const std::string& name, const std::any& value);
    std::any Get(const Token& name) const;
    void Assign(const Token& name, const std::any& value);
    std::any GetAt(int distance, const std::string& name);
    void AssignAt(int distance, const Token& name, const std::any& value);
    Environment* Ancestor(int distance);
    std::shared_ptr<Environment> GetEnclosing();

private:
    std::unordered_map<std::string, std::any> m_Variables;
    std::shared_ptr<Environment> m_Enclosing;
};