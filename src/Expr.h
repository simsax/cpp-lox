#pragma once
#include <any>
#include <memory>
#include "Token.h"


namespace expr {

	class Visitor;

	struct Expr {
		virtual ~Expr() = 0;

		virtual std::any Accept(Visitor& visitor) = 0;
	};

	inline Expr::~Expr() = default;

	struct Binary : public Expr {
		Binary(std::unique_ptr<Expr> left, const Token& opr, std::unique_ptr<Expr> right) :
			m_Left(std::move(left)),
			m_Opr(opr),
			m_Right(std::move(right))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::unique_ptr<Expr> m_Left;
		Token m_Opr;
		std::unique_ptr<Expr> m_Right;
	};

	struct Grouping : public Expr {
		Grouping(std::unique_ptr<Expr> expression) :
			m_Expression(std::move(expression))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::unique_ptr<Expr> m_Expression;
	};

	struct Literal : public Expr {
		Literal(const std::any& value) :
			m_Value(value)
		{ }

		std::any Accept(Visitor& visitor) override;

		std::any m_Value;
	};

	struct Unary : public Expr {
		Unary(const Token& opr, std::unique_ptr<Expr> right) :
			m_Opr(opr),
			m_Right(std::move(right))
		{ }

		std::any Accept(Visitor& visitor) override;

		Token m_Opr;
		std::unique_ptr<Expr> m_Right;
	};

	struct Variable : public Expr {
		Variable(const Token& name) :
			m_Name(name)
		{ }

		std::any Accept(Visitor& visitor) override;

		Token m_Name;
	};

	class Visitor {
	public:
		virtual ~Visitor() = 0;
		virtual std::any VisitBinary(Binary* expr) = 0;
		virtual std::any VisitGrouping(Grouping* expr) = 0;
		virtual std::any VisitLiteral(Literal* expr) = 0;
		virtual std::any VisitUnary(Unary* expr) = 0;
		virtual std::any VisitVariable(Variable* expr) = 0;
	};

	inline Visitor::~Visitor() = default;

	inline std::any Binary::Accept(Visitor& visitor) {
		return visitor.VisitBinary(this);
	}

	inline std::any Grouping::Accept(Visitor& visitor) {
		return visitor.VisitGrouping(this);
	}

	inline std::any Literal::Accept(Visitor& visitor) {
		return visitor.VisitLiteral(this);
	}

	inline std::any Unary::Accept(Visitor& visitor) {
		return visitor.VisitUnary(this);
	}

	inline std::any Variable::Accept(Visitor& visitor) {
		return visitor.VisitVariable(this);
	}

}
