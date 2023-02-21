#include "AstPrinter.h"
#include <variant>
#include <cstdarg>

AstPrinter::AstPrinter()
{
}

std::string AstPrinter::Print(Expr<std::string>* expr) {
	return expr->Accept(this);
}

std::string AstPrinter::VisitBinaryExpr(Binary<std::string>* expr)
{
	return Parenthesize(expr->m_Opr.lexeme, expr->m_Left, expr->m_Right);
}

std::string AstPrinter::VisitGroupingExpr(Grouping<std::string>* expr)
{
	return Parenthesize("group", expr->m_Expression);
}

std::string AstPrinter::VisitLiteralExpr(Literal<std::string>* expr)
{
	if (std::holds_alternative<std::monostate>(expr->m_Value))
		return "nil";
	else if (std::holds_alternative<std::string>(expr->m_Value))
		return std::get<std::string>(expr->m_Value);
	else
		return std::to_string(std::get<double>(expr->m_Value));
}

std::string AstPrinter::VisitUnaryExpr(Unary<std::string>* expr)
{
	return Parenthesize(expr->m_Opr.lexeme, expr->m_Right);
}

