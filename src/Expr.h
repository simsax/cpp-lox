#pragma once
#include <any>
#include <memory>
#include "Token.h"

class Visitor;

struct Expr {
    virtual ~Expr() = 0;

    virtual std::any Accept(const Visitor& visitor) = 0;
};

inline Expr::~Expr() = default;

struct BinaryExpr : public Expr {
    BinaryExpr(std::unique_ptr<Expr> left, const Token& opr, std::unique_ptr<Expr> right) :
	m_Left(std::move(left)),
	m_Opr(opr),
	m_Right(std::move(right))
	{ }

    std::any Accept(const Visitor& visitor) override;

	std::unique_ptr<Expr> m_Left;
	Token m_Opr;
	std::unique_ptr<Expr> m_Right;
};

struct GroupingExpr : public Expr {
    GroupingExpr(std::unique_ptr<Expr> expression) :
	m_Expression(std::move(expression))
	{ }

    std::any Accept(const Visitor& visitor) override;

	std::unique_ptr<Expr> m_Expression;
};

struct LiteralExpr : public Expr {
    LiteralExpr(const std::any& value) :
	m_Value(value)
	{ }

    std::any Accept(const Visitor& visitor) override;

	std::any m_Value;
};

struct UnaryExpr : public Expr {
    UnaryExpr(const Token& opr, std::unique_ptr<Expr> right) :
	m_Opr(opr),
	m_Right(std::move(right))
	{ }

    std::any Accept(const Visitor& visitor) override;

	Token m_Opr;
	std::unique_ptr<Expr> m_Right;
};

class Visitor {
public:
    virtual ~Visitor() = 0;
	virtual std::any VisitBinaryExpr(BinaryExpr* expr) const = 0;
	virtual std::any VisitGroupingExpr(GroupingExpr* expr) const = 0;
	virtual std::any VisitLiteralExpr(LiteralExpr* expr) const = 0;
	virtual std::any VisitUnaryExpr(UnaryExpr* expr) const = 0;
};

inline Visitor::~Visitor() = default;

inline std::any BinaryExpr::Accept(const Visitor& visitor) {
	return visitor.VisitBinaryExpr(this);          
}

inline std::any GroupingExpr::Accept(const Visitor& visitor) {
	return visitor.VisitGroupingExpr(this);          
}

inline std::any LiteralExpr::Accept(const Visitor& visitor) {
	return visitor.VisitLiteralExpr(this);          
}

inline std::any UnaryExpr::Accept(const Visitor& visitor) {
	return visitor.VisitUnaryExpr(this);          
}
