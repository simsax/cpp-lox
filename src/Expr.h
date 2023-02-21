#pragma once
#include <variant>
#include "Token.h"

template<typename T>
class Visitor;

template<typename T>
struct Expr {
	virtual ~Expr() = 0;

	virtual T Accept(Visitor<T>* visitor) = 0;
};

template<typename T>
inline Expr<T>::~Expr() = default;

template<typename T>
struct Binary : public Expr<T> {
	Binary(Expr<T>* left, const Token& opr, Expr<T>* right) :
		m_Left(left),
		m_Opr(opr),
		m_Right(right)
	{ }

	inline T Accept(Visitor<T>* visitor) override {
		return visitor->VisitBinaryExpr(this);
	}

	Expr<T>* m_Left;
	Token m_Opr;
	Expr<T>* m_Right;
};

template<typename T>
struct Grouping : public Expr<T> {
	Grouping(Expr<T>* expression) :
		m_Expression(expression)
	{ }

	inline T Accept(Visitor<T>* visitor) override {
		return visitor->VisitGroupingExpr(this);
	}

	Expr<T>* m_Expression;
};

template<typename T>
struct Literal : public Expr<T> {
	Literal(const std::variant<std::monostate, double, std::string>& value) :
		m_Value(value)
	{ }

	inline T Accept(Visitor<T>* visitor) override {
		return visitor->VisitLiteralExpr(this);
	}

	std::variant<std::monostate, double, std::string> m_Value;
};

template<typename T>
struct Unary : public Expr<T> {
	Unary(const Token& opr, Expr<T>* right) :
		m_Opr(opr),
		m_Right(right)
	{ }

	inline T Accept(Visitor<T>* visitor) override {
		return visitor->VisitUnaryExpr(this);
	}

	Token m_Opr;
	Expr<T>* m_Right;
};

template<typename T>
class Visitor {
public:
	virtual ~Visitor() = 0;
	virtual T VisitBinaryExpr(Binary<T>* expr) = 0;
	virtual T VisitGroupingExpr(Grouping<T>* expr) = 0;
	virtual T VisitLiteralExpr(Literal<T>* expr) = 0;
	virtual T VisitUnaryExpr(Unary<T>* expr) = 0;
};

template<typename T>
inline Visitor<T>::~Visitor() = default;