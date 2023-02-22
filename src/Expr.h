#pragma once
#include <any>
#include "Token.h"

class Visitor;

struct Expr {
    virtual ~Expr() = 0;

    virtual std::any Accept(Visitor* visitor) = 0;
};

inline Expr::~Expr() = default;

struct Binary : public Expr {
    Binary(Expr* left, const Token& opr, Expr* right) :
	m_Left(left),
	m_Opr(opr),
	m_Right(right)
	{ }

    std::any Accept(Visitor* visitor) override;

	Expr* m_Left;
	Token m_Opr;
	Expr* m_Right;
};

struct Grouping : public Expr {
    Grouping(Expr* expression) :
	m_Expression(expression)
	{ }

    std::any Accept(Visitor* visitor) override;

	Expr* m_Expression;
};

struct Literal : public Expr {
    Literal(const std::any& value) :
	m_Value(value)
	{ }

    std::any Accept(Visitor* visitor) override;

	std::any m_Value;
};

struct Unary : public Expr {
    Unary(const Token& opr, Expr* right) :
	m_Opr(opr),
	m_Right(right)
	{ }

    std::any Accept(Visitor* visitor) override;

	Token m_Opr;
	Expr* m_Right;
};

class Visitor {
public:
    virtual ~Visitor() = 0;
	virtual std::any VisitBinaryExpr(Binary* expr) = 0;
	virtual std::any VisitGroupingExpr(Grouping* expr) = 0;
	virtual std::any VisitLiteralExpr(Literal* expr) = 0;
	virtual std::any VisitUnaryExpr(Unary* expr) = 0;
};

inline Visitor::~Visitor() = default;

inline std::any Binary::Accept(Visitor* visitor) {
	return visitor->VisitBinaryExpr(this);          
}

inline std::any Grouping::Accept(Visitor* visitor) {
	return visitor->VisitGroupingExpr(this);          
}

inline std::any Literal::Accept(Visitor* visitor) {
	return visitor->VisitLiteralExpr(this);          
}

inline std::any Unary::Accept(Visitor* visitor) {
	return visitor->VisitUnaryExpr(this);          
}
