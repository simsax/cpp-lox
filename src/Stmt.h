#pragma once
#include <any>
#include <memory>
#include "Token.h"
#include "Expr.h"

namespace stmt {

	class Visitor;

	struct Stmt {
		virtual ~Stmt() = 0;

		virtual std::any Accept(const Visitor& visitor) = 0;
	};

	inline Stmt::~Stmt() = default;

	struct Expression : public Stmt {
		Expression(std::unique_ptr<expr::Expr> expression) :
			m_Expression(std::move(expression))
		{ }

		std::any Accept(const Visitor& visitor) override;

		std::unique_ptr<expr::Expr> m_Expression;
	};

	struct Print : public Stmt {
		Print(std::unique_ptr<expr::Expr> expression) :
			m_Expression(std::move(expression))
		{ }

		std::any Accept(const Visitor& visitor) override;

		std::unique_ptr<expr::Expr> m_Expression;
	};

	class Visitor {
	public:
		virtual ~Visitor() = 0;
		virtual std::any VisitExpression(Expression* stmt) const = 0;
		virtual std::any VisitPrint(Print* stmt) const = 0;
	};

	inline Visitor::~Visitor() = default;

	inline std::any Expression::Accept(const Visitor& visitor) {
		return visitor.VisitExpression(this);
	}

	inline std::any Print::Accept(const Visitor& visitor) {
		return visitor.VisitPrint(this);
	}

}
