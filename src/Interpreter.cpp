#include "Interpreter.h"
#include "Lox.h"
#include <regex>

Interpreter::Interpreter()
{
}


void Interpreter::Interpret(const std::vector<std::unique_ptr<stmt::Stmt>>& statements) const
{
	try {
		for (const auto& statement : statements) {
			Execute(statement.get());
		}
	}
	catch (const RuntimeException& ex) {
		Lox::RuntimeError(ex.GetToken(), ex.what());
	}
}

std::any Interpreter::VisitBinary(expr::Binary* expr) const
{
	std::any left = Evaluate(expr->m_Left.get());
	std::any right = Evaluate(expr->m_Right.get());

	switch (expr->m_Opr.type) {
	case TokenType::MINUS:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) - std::any_cast<double>(right);
	case TokenType::PLUS:
		if (left.type() == typeid(double) && right.type() == typeid(double))
			return std::any_cast<double>(left) + std::any_cast<double>(right);
		else if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
			return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
		else
			throw RuntimeException("Operands must be two numbers or two strings.", expr->m_Opr);
	case TokenType::SLASH:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) / std::any_cast<double>(right);
	case TokenType::STAR:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) * std::any_cast<double>(right);
	case TokenType::GREATER:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) > std::any_cast<double>(right);
	case TokenType::GREATER_EQUAL:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) >= std::any_cast<double>(right);
	case TokenType::LESS:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) < std::any_cast<double>(right);
	case TokenType::LESS_EQUAL:
		CheckNumberOperands(expr->m_Opr, left, right);
		return std::any_cast<double>(left) <= std::any_cast<double>(right);
	case TokenType::EQUAL_EQUAL:
		return IsEqual(left, right);
	case TokenType::BANG_EQUAL:
		return !IsEqual(left, right);
	default:
		break;
	}

	// unreachable
	return nullptr;
}

std::any Interpreter::VisitGrouping(expr::Grouping* expr) const
{
	return Evaluate(expr->m_Expression.get());
}

std::any Interpreter::VisitLiteral(expr::Literal* expr) const
{
	return expr->m_Value;
}

std::any Interpreter::VisitUnary(expr::Unary* expr) const
{
	// post order traversal: each node evaluates the children before doing its own work
	std::any right = Evaluate(expr->m_Right.get());
	switch (expr->m_Opr.type) {
	case TokenType::BANG:
		return !(IsTruthy(right));
	case TokenType::MINUS:
		CheckNumberOperand(expr->m_Opr, right);
		return -std::any_cast<double>(right);
	default:
		break;
	}
	// unreachable
	return nullptr;
}

std::any Interpreter::VisitExpression(stmt::Expression* stmt) const
{
	Evaluate(stmt->m_Expression.get());
	return nullptr;
}

std::any Interpreter::VisitPrint(stmt::Print* stmt) const
{
	std::any value = Evaluate(stmt->m_Expression.get());
	std::cout << ToString(value) << "\n";
	return nullptr;
}

void Interpreter::CheckNumberOperand(const Token& opr, const std::any& operand) const
{
	if (operand.type() == typeid(double))
		return;
	throw RuntimeException("Operand must be a number.", opr);
}

void Interpreter::CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const
{
	if (left.type() == typeid(double) && right.type() == typeid(double))
		return;
	throw RuntimeException("Operands must be numbers.", opr);
}

std::any Interpreter::Evaluate(expr::Expr* expr) const
{
	return expr->Accept(*this);
}

void Interpreter::Execute(stmt::Stmt* statement) const
{
	statement->Accept(*this);
}

bool Interpreter::IsTruthy(std::any value) const
{
	if (value.type() == typeid(bool))
		return std::any_cast<bool>(value);
	if (value.type() == typeid(std::nullptr_t))
		return false;
	return true;
}

bool Interpreter::IsEqual(std::any left, std::any right) const
{
	if (left.type() != right.type())
		return false;
	if (left.type() == typeid(bool) && right.type() == typeid(bool))
		return std::any_cast<bool>(left) == std::any_cast<bool>(right);
	if (left.type() == typeid(double) && right.type() == typeid(double))
		return std::any_cast<double>(left) == std::any_cast<double>(right);
	if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
		return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
	if (left.type() == typeid(std::nullptr_t) && right.type() == typeid(std::nullptr_t))
		return true;
	return false;
}

std::string Interpreter::ToString(const std::any& value) const
{
	if (value.type() == typeid(int)) {
		int val = std::any_cast<int>(value);
		return std::to_string(val);
	}
	else if (value.type() == typeid(double)) {
		double val = std::any_cast<double>(value);
		std::string doubleString = std::to_string(val);
		return std::regex_replace(doubleString, std::regex{ "\\.0+$" }, "");
	}
	else if (value.type() == typeid(bool)) {
		bool val = std::any_cast<bool>(value);
		if (val)
			return "true";
		else
			return "false";
	}
	else if (value.type() == typeid(nullptr)) {
		return "null";
	}
	else {
		return std::any_cast<std::string>(value);
	}
}

