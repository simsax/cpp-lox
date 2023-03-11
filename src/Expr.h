#pragma once
#include <any>
#include <memory>
#include "Token.h"

namespace expr {

	class Visitor;

	struct Expr {
		virtual ~Expr() = 0;

		virtual std::any Accept(const Visitor& visitor) = 0;
	};

	inline Expr::~Expr() = default;

	struct Binary : public Expr {
		Binary(std::unique_ptr<Expr> left, const Token& opr, std::unique_ptr<Expr> right) :
			m_Left(std::move(left)),
			m_Opr(opr),
			m_Right(std::move(right))
		{ }

		std::any Accept(const Visitor& visitor) override;

		std::unique_ptr<Expr> m_Left;
		Token m_Opr;
		std::unique_ptr<Expr> m_Right;
	};

	struct Grouping : public Expr {
		Grouping(std::unique_ptr<Expr> expression) :
			m_Expression(std::move(expression))
		{ }

		std::any Accept(const Visitor& visitor) override;

		std::unique_ptr<Expr> m_Expression;
	};

	struct Literal : public Expr {
		Literal(const std::any& value) :
			m_Value(value)
		{ }

		std::any Accept(const Visitor& visitor) override;

		std::any m_Value;
	};

	struct Unary : public Expr {
		Unary(const Token& opr, std::unique_ptr<Expr> right) :
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
		virtual std::any VisitBinary(Binary* expr) const = 0;
		virtual std::any VisitGrouping(Grouping* expr) const = 0;
		virtual std::any VisitLiteral(Literal* expr) const = 0;
		virtual std::any VisitUnary(Unary* expr) const = 0;
	};

	inline Visitor::~Visitor() = default;

	inline std::any Binary::Accept(const Visitor& visitor) {
		return visitor.VisitBinary(this);
	}

	inline std::any Grouping::Accept(const Visitor& visitor) {
		return visitor.VisitGrouping(this);
	}

	inline std::any Literal::Accept(const Visitor& visitor) {
		return visitor.VisitLiteral(this);
	}

	inline std::any Unary::Accept(const Visitor& visitor) {
		return visitor.VisitUnary(this);
	}

}
