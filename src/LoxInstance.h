#pragma once

#include "Token.h"
#include <memory>
#include <unordered_map>

class LoxClass;

class LoxInstance : public std::enable_shared_from_this<LoxInstance> {
public:
	LoxInstance(std::shared_ptr<LoxClass> classPtr);
	virtual ~LoxInstance() = default;
	std::any Get(const Token& name);
	void Set(const Token& name, const std::any& value);
	std::string ToString() const;

protected:
	template <typename Derived>
	std::shared_ptr<Derived> SharedFromBase() {
		return std::static_pointer_cast<Derived>(shared_from_this());
	}

private:
	std::shared_ptr<LoxClass> m_Class;
	std::unordered_map<std::string, std::any> m_Fields;
};