#pragma once
#include <stdexcept>
#include "Expr.h"

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

class Interpreter : public Visitor {
public:
	Interpreter();

	void Interpret(Expr* expr);

	std::any VisitBinaryExpr(BinaryExpr* expr) const override;
	std::any VisitGroupingExpr(GroupingExpr* expr) const override;
	std::any VisitLiteralExpr(LiteralExpr* expr) const override;
	std::any VisitUnaryExpr(UnaryExpr* expr) const override;
private:
	void CheckNumberOperand(const Token& opr, const std::any& operand) const;
	void CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const;
	std::string ToString(const std::any& value) const;

	std::any Evaluate(Expr* expr) const;
	bool IsTruthy(std::any value) const;
	bool IsEqual(std::any left, std::any right) const;
};