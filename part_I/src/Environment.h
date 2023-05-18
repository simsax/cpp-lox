#pragma once
#include <stdint.h>
#include <unordered_map>
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include "Token.h"


class Environment {
public:
	Environment();
	Environment(std::shared_ptr<Environment> enclosing);
	void Define(const std::string& name, const std::any& value);
	void DefineLocal(const std::any& value);
	std::any Get(const Token& name) const;
	void Assign(const Token& name, const std::any& value);
	std::any GetAt(uint32_t distance, uint32_t variableIndex);
	void AssignAt(uint32_t distance, uint32_t variableIndex, const std::any& value);
	Environment* Ancestor(int distance);
	std::shared_ptr<Environment> GetEnclosing();

private:
	std::shared_ptr<Environment> m_Enclosing;
	std::unordered_map<std::string, std::any> m_Globals;
	std::vector<std::any> m_Locals;
};