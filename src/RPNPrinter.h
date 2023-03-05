#pragma once
#include "Expr.h"
#include <string>

class RPNPrinter : public Visitor {
public:
	RPNPrinter();

	std::string Print(Expr* expr) const;
	std::any VisitBinaryExpr(BinaryExpr* expr) const override;
	std::any VisitGroupingExpr(GroupingExpr* expr) const override;
	std::any VisitLiteralExpr(LiteralExpr* expr) const override;
	std::any VisitUnaryExpr(UnaryExpr* expr) const override;

private:
	template<typename ...Args>
	std::string Parenthesize(const std::string& name, Args... exprs) const;
};

template<typename ...Args>
inline std::string RPNPrinter::Parenthesize(const std::string& name, Args ...exprs) const
{
	std::string tree = "";
	for (Expr* child : { exprs... }) {
		std::any result = child->Accept(*this);
		if (result.type() == typeid(int)) {
			int value = std::any_cast<int>(result);
			tree += std::to_string(value);
		}
		else if (result.type() == typeid(double)) {
			double value = std::any_cast<double>(result);
			tree += std::to_string(value);
		}
		else if (result.type() == typeid(bool)) {
			bool value = std::any_cast<bool>(result);
			tree += std::to_string(value);
		}
		else if (result.type() == typeid(nullptr)) {
			tree += "null";
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
