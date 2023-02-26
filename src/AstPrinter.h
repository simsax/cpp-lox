#pragma once
#include "Expr.h"
#include <string>

class AstPrinter : public Visitor {
public:
	AstPrinter();

	std::string Print(Expr* expr);
	std::any VisitBinaryExpr(BinaryExpr* expr) override;
	std::any VisitGroupingExpr(GroupingExpr* expr) override;
	std::any VisitLiteralExpr(LiteralExpr* expr) override;
	std::any VisitUnaryExpr(UnaryExpr* expr) override;

private:
	template<typename ...Args>
	std::string Parenthesize(const std::string& name, Args... exprs);
};

template<typename ...Args>
inline std::string AstPrinter::Parenthesize(const std::string& name, Args ...exprs)
{
	std::string tree = "";
	tree += "(" + name;
	for (Expr* child : { exprs... }) {
		tree += " ";
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
	}
	tree += ")";
	return tree;
}
