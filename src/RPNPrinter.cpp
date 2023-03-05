#include "RPNPrinter.h"

RPNPrinter::RPNPrinter()
{
}

std::string RPNPrinter::Print(Expr* expr) const {
	std::any result = expr->Accept(*this);

	if (result.type() == typeid(int)) {
		int value = std::any_cast<int>(result);
		return std::to_string(value);
	}
	else if (result.type() == typeid(double)) {
		double value = std::any_cast<double>(result);
		return std::to_string(value);
	}
	else if (result.type() == typeid(bool)) {
		bool value = std::any_cast<bool>(result);
		return std::to_string(value);
	}
	else if (result.type() == typeid(nullptr)) {
		return "null";
	}
	else {
		return std::any_cast<std::string>(result);
	}

}

std::any RPNPrinter::VisitBinaryExpr(BinaryExpr* expr) const
{
	return Parenthesize(expr->m_Opr.lexeme, expr->m_Left.get(), expr->m_Right.get());
}

std::any RPNPrinter::VisitGroupingExpr(GroupingExpr* expr) const
{
	return Parenthesize("group", expr->m_Expression.get());
}

std::any RPNPrinter::VisitLiteralExpr(LiteralExpr* expr) const
{
	if (!expr->m_Value.has_value())
		return "nil";
	else
		return expr->m_Value;
}

std::any RPNPrinter::VisitUnaryExpr(UnaryExpr* expr) const
{
	return Parenthesize(expr->m_Opr.lexeme, expr->m_Right.get());
}
