#pragma once
#include <unordered_map>
#include <any>
#include <string>
#include "LoxCallable.h"
#include "Token.h"

class Environment {
public:
	Environment();
	Environment(Environment* enclosing);
	void Define(const std::string& name, const std::any& value);
	void Assign(const Token& name, const std::any& value);
	std::any Get(const Token& name) const;

private:
	std::unordered_map<std::string, std::any> m_Variables;
	Environment* m_Enclosing;
};