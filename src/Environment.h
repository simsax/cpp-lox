#pragma once
#include <unordered_map>
#include <any>
#include <string>
#include "Token.h"

class Environment {
public:
	Environment();
	void Define(const Token& name, const std::any& value);
	void Assign(const Token& name, const std::any& value);
	std::any Get(const Token& name) const;

private:
	std::unordered_map<std::string, std::any> m_Variables;
};