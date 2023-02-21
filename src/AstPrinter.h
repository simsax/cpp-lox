#pragma once
#include "Expr.h"
#include <string>

class AstPrinter : public Visitor<std::string> {
public:
	AstPrinter();

	std::string Print(Expr<std::string>* expr);
	std::string VisitBinaryExpr(Binary<std::string>* expr) override;
	std::string VisitGroupingExpr(Grouping<std::string>* expr) override;
	std::string VisitLiteralExpr(Literal<std::string>* expr) override;
	std::string VisitUnaryExpr(Unary<std::string>* expr) override;

private:
	template<typename... Args>
	std::string Parenthesize(const std::string& name, Args... exprs);
};

template<typename ...Args>
inline std::string AstPrinter::Parenthesize(const std::string& name, Args ...exprs)
{
	std::string tree = "";
	tree += "(" + name;
	for (Expr<std::string>* expr : { exprs... }) {
		tree += " ";
		tree += expr->Accept(this);
	}
	tree += ")";
	return tree;
}
