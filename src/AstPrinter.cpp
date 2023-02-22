#include "AstPrinter.h"

AstPrinter::AstPrinter()
{
}

std::string AstPrinter::Print(Expr* expr) {
	std::any result = expr->Accept(this);

	if (result.type() == typeid(int)) {
		int value = std::any_cast<int>(result);
		return std::to_string(value);
	}
	else if (result.type() == typeid(double)) {
		double value = std::any_cast<double>(result);
		return std::to_string(value);
	}
	else {
		return std::any_cast<std::string>(result);
	}
}

std::any AstPrinter::VisitBinaryExpr(Binary* expr)
{
	return Parenthesize(expr->m_Opr.lexeme, expr->m_Left, expr->m_Right);
}

std::any AstPrinter::VisitGroupingExpr(Grouping* expr)
{
	return Parenthesize("group", expr->m_Expression);
}

std::any AstPrinter::VisitLiteralExpr(Literal* expr)
{
	if (!expr->m_Value.has_value())
		return "nil";
	else
		return expr->m_Value;
}

std::any AstPrinter::VisitUnaryExpr(Unary* expr)
{
	return Parenthesize(expr->m_Opr.lexeme, expr->m_Right);
}

