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

	struct If : public Stmt {
		If(std::unique_ptr<expr::Expr> condition, std::unique_ptr<stmt::Stmt> thenBranch,
			std::unique_ptr<stmt::Stmt> elseBranch) :
			m_Condition(std::move(condition)),
			m_ThenBranch(std::move(thenBranch)),
			m_ElseBranch(std::move(elseBranch))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::unique_ptr<expr::Expr> m_Condition;
		std::unique_ptr<stmt::Stmt> m_ThenBranch;
		std::unique_ptr<stmt::Stmt> m_ElseBranch;
	};

	struct While : public Stmt {
		While(std::unique_ptr<expr::Expr> condition, std::unique_ptr<stmt::Stmt> statement) :
			m_Condition(std::move(condition)),
			m_Statement(std::move(statement))
		{ }

		std::any Accept(Visitor& visitor) override;

		std::unique_ptr<expr::Expr> m_Condition;
		std::unique_ptr<stmt::Stmt> m_Statement;
	};

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

	struct Function : public Stmt {
		Function(const Token& name,
			const std::vector<Token>& params,
			std::vector<std::unique_ptr<stmt::Stmt>> body) :
			m_Name(name),
			m_Params(params),
			m_Body(std::move(body))
		{ }

		std::any Accept(Visitor& visitor) override;

		Token m_Name;
		std::vector<Token> m_Params;
		std::vector<std::unique_ptr<stmt::Stmt>> m_Body;
	};

	struct Return : public Stmt {
		Return(const Token& keyword, std::unique_ptr<expr::Expr> expression) :
			m_Keyword(keyword), m_Expression(std::move(expression))
		{}

		std::any Accept(Visitor& visitor) override;

		Token m_Keyword;
		std::unique_ptr<expr::Expr> m_Expression;
	};

	class Visitor {
	public:
		virtual ~Visitor() = 0;
		virtual std::any VisitBlock(Block* stmt) = 0;
		virtual std::any VisitExpression(Expression* stmt) = 0;
		virtual std::any VisitPrint(Print* stmt) = 0;
		virtual std::any VisitVar(Var* stmt) = 0;
		virtual std::any VisitIf(If* stmt) = 0;
		virtual std::any VisitWhile(While* stmt) = 0;
		virtual std::any VisitFunction(Function* stmt) = 0;
		virtual std::any VisitReturn(Return* stmt) = 0;
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

	inline std::any If::Accept(Visitor& visitor) {
		return visitor.VisitIf(this);
	}

	inline std::any While::Accept(Visitor& visitor) {
		return visitor.VisitWhile(this);
	}

	inline std::any Function::Accept(Visitor& visitor) {
		return visitor.VisitFunction(this);
	}

	inline std::any Return::Accept(Visitor& visitor) {
		return visitor.VisitReturn(this);
	}
}
