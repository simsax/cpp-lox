#pragma once
#include "Expr.h"
#include <string>

class RPNPrinter : public Visitor {
public:
	RPNPrinter();

	std::string Print(Expr* expr);
	std::any VisitBinaryExpr(Binary* expr) override;
	std::any VisitGroupingExpr(Grouping* expr) override;
	std::any VisitLiteralExpr(Literal* expr) override;
	std::any VisitUnaryExpr(Unary* expr) override;

private:
	template<typename ...Args>
	std::string Parenthesize(const std::string& name, Args... exprs);
};

template<typename ...Args>
inline std::string RPNPrinter::Parenthesize(const std::string& name, Args ...exprs)
{
	std::string tree = "";
	for (Expr* child : { exprs... }) {
		std::any result = child->Accept(this);
		if (result.type() == typeid(int)) {
			int value = std::any_cast<int>(result);
			tree += std::to_string(value);
		}
		else if (result.type() == typeid(double)) {
			double value = std::any_cast<double>(result);
			tree += std::to_string(value);
		}
		else {
			std::string value = std::any_cast<std::string>(result);
			tree += value;
		}
		tree += " ";
	}
	tree += name;
	return tree;
}
