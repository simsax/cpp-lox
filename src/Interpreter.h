#pragma once
#include <stdexcept>
#include <vector>
#include "Expr.h"
#include "Stmt.h"

class RuntimeException : public std::runtime_error {
public:
	RuntimeException(const std::string& error, const Token& token) :
		std::runtime_error(error.c_str()), m_Token(token)
	{
	}

	inline const Token& GetToken() const {
		return m_Token;
	}

private:
	Token m_Token;
};

class Interpreter : public expr::Visitor, public stmt::Visitor {
public:
	Interpreter();

	void Interpret(const std::vector<std::unique_ptr<stmt::Stmt>>& statements) const;

	std::any VisitBinary(expr::Binary* expr) const override;
	std::any VisitGrouping(expr::Grouping* expr) const override;
	std::any VisitLiteral(expr::Literal* expr) const override;
	std::any VisitUnary(expr::Unary* expr) const override;

	std::any VisitExpression(stmt::Expression* stmt) const override;
	std::any VisitPrint(stmt::Print* stmt) const override;
private:
	void CheckNumberOperand(const Token& opr, const std::any& operand) const;
	void CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const;
	std::string ToString(const std::any& value) const;

	std::any Evaluate(expr::Expr* expr) const;
	void Execute(stmt::Stmt* statement) const;
	bool IsTruthy(std::any value) const;
	bool IsEqual(std::any left, std::any right) const;
};