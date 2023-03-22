#pragma once
#include <stdexcept>
#include <vector>
#include "Expr.h"
#include "Stmt.h"
#include "Environment.h"

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
	~Interpreter();
	Interpreter(const Interpreter&) = delete;
	Interpreter& operator=(const Interpreter&) = delete;

	void Interpret(const std::vector<std::unique_ptr<stmt::Stmt>>& statements);

	std::any VisitAssign(expr::Assign* expr) override;
	std::any VisitBinary(expr::Binary* expr) override;
	std::any VisitLogical(expr::Logical* expr) override;
	std::any VisitGrouping(expr::Grouping* expr) override;
	std::any VisitLiteral(expr::Literal* expr) override;
	std::any VisitUnary(expr::Unary* expr) override;
	std::any VisitVariable(expr::Variable* expr) override;

	std::any VisitExpression(stmt::Expression* stmt) override;
	std::any VisitPrint(stmt::Print* stmt) override;
	std::any VisitVar(stmt::Var* stmt) override;
	std::any VisitBlock(stmt::Block* stmt) override;
	std::any VisitIf(stmt::If* stmt) override;
	std::any VisitWhile(stmt::While* stmt) override;

private:
	void CheckNumberOperand(const Token& opr, const std::any& operand) const;
	void CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const;
	std::string ToString(const std::any& value) const;

	std::any Evaluate(expr::Expr* expr);
	void Execute(stmt::Stmt* statement);
	void ExecuteBlock(const std::vector<std::unique_ptr<stmt::Stmt>>& statements);
	bool IsTruthy(std::any value) const;
	bool IsEqual(std::any left, std::any right) const;

	Environment* m_CurrentEnvironment;
};