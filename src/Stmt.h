#pragma once
#include <any>
#include <memory>
#include <vector>
#include "Token.h"
#include "Expr.h"

namespace stmt {

	class Visitor;

	struct Stmt {
		virtual ~Stmt() = 0;

		virtual std::any Accept(Visitor& visitor) = 0;
	};

	inline Stmt::~Stmt() = default;

	struct Block : public Stmt {
		Block(std::vector<std::unique_ptr<stmt::Stmt>> statements) :
			m_Statements(std::move(statements))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::vector<std::unique_ptr<stmt::Stmt>> m_Statements;
	};

	struct Expression : public Stmt {
		Expression(std::unique_ptr<expr::Expr> expression) :
			m_Expression(std::move(expression))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::unique_ptr<expr::Expr> m_Expression;
	};

	struct Print : public Stmt {
		Print(std::unique_ptr<expr::Expr> expression) :
			m_Expression(std::move(expression))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::unique_ptr<expr::Expr> m_Expression;
	};

	struct Var : public Stmt {
		Var(const Token& name, std::unique_ptr<expr::Expr> initializer) :
			m_Name(name),
			m_Initializer(std::move(initializer))
		{ }

		std::any Accept(Visitor& visitor) override;

		Token m_Name;
		std::unique_ptr<expr::Expr> m_Initializer;
	};

	class Visitor {
	public:
		virtual ~Visitor() = 0;
		virtual std::any VisitBlock(Block* stmt) = 0;
		virtual std::any VisitExpression(Expression* stmt) = 0;
		virtual std::any VisitPrint(Print* stmt) = 0;
		virtual std::any VisitVar(Var* stmt) = 0;
	};

	inline Visitor::~Visitor() = default;

	inline std::any Block::Accept(Visitor& visitor) {
		return visitor.VisitBlock(this);
	}

	inline std::any Expression::Accept(Visitor& visitor) {
		return visitor.VisitExpression(this);
	}

	inline std::any Print::Accept(Visitor& visitor) {
		return visitor.VisitPrint(this);
	}

	inline std::any Var::Accept(Visitor& visitor) {
		return visitor.VisitVar(this);
	}

}
