#pragma once
#include <any>
#include "Token.h"

class Visitor;

struct Expr {
    virtual ~Expr() = 0;

    virtual std::any Accept(Visitor* visitor) = 0;
};

inline Expr::~Expr() = default;

struct BinaryExpr : public Expr {
    BinaryExpr(Expr* left, const Token& opr, Expr* right) :
	m_Left(left),
	m_Opr(opr),
	m_Right(right)
	{ }

    std::any Accept(Visitor* visitor) override;

	Expr* m_Left;
	Token m_Opr;
	Expr* m_Right;
};

struct GroupingExpr : public Expr {
    GroupingExpr(Expr* expression) :
	m_Expression(expression)
	{ }

    std::any Accept(Visitor* visitor) override;

	Expr* m_Expression;
};

struct LiteralExpr : public Expr {
    LiteralExpr(const std::any& value) :
	m_Value(value)
	{ }

    std::any Accept(Visitor* visitor) override;

	std::any m_Value;
};

struct UnaryExpr : public Expr {
    UnaryExpr(const Token& opr, Expr* right) :
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
	virtual std::any VisitBinaryExpr(BinaryExpr* expr) = 0;
	virtual std::any VisitGroupingExpr(GroupingExpr* expr) = 0;
	virtual std::any VisitLiteralExpr(LiteralExpr* expr) = 0;
	virtual std::any VisitUnaryExpr(UnaryExpr* expr) = 0;
};

inline Visitor::~Visitor() = default;

inline std::any BinaryExpr::Accept(Visitor* visitor) {
	return visitor->VisitBinaryExpr(this);          
}

inline std::any GroupingExpr::Accept(Visitor* visitor) {
	return visitor->VisitGroupingExpr(this);          
}

inline std::any LiteralExpr::Accept(Visitor* visitor) {
	return visitor->VisitLiteralExpr(this);          
}

inline std::any UnaryExpr::Accept(Visitor* visitor) {
	return visitor->VisitUnaryExpr(this);          
}
