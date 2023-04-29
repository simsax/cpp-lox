#pragma once

#include "LoxFunction.h"
#include "LoxInstance.h"
#include <string>

using MethodMap = std::unordered_map<std::string, std::shared_ptr<LoxFunction>>;

class LoxClass : public LoxCallable, public LoxInstance {
public:
	LoxClass(const std::string& name, MethodMap methods, std::shared_ptr<LoxClass> metaClass);
	virtual ~LoxClass();

	std::any Call(Interpreter& interpreter, const std::vector<std::any>& arguments) override;
	size_t Arity() const override;
	std::string ToString() const override;
	std::string GetName() const;
	std::shared_ptr<LoxFunction> FindMethod(const std::string& name) const;

private:
	std::string m_Name;
	MethodMap m_Methods;
};