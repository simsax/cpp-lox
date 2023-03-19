#pragma once
#include <vector>
#include <concepts>
#include <exception>
#include <stdexcept>
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"

/*
// Statements
program        → declaration* EOF ;
declaration	   → varDecl | statement ;
varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
statement      → exprStmt | printStmt | block ;
exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
block          → "{" declaration* "}" ;

// Expressions
comma		   → expression ( "," expression )* ;
expression     → equality ;
assignment     → IDENTIFIER "=" assignment | equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
			   | "(" expression ")" | IDENTIFIER ;
*/

// TODO: ternary operator

template<typename T>
concept IsTokenType = std::is_same<T, TokenType>::value;

class ParseException : public std::runtime_error {
public:
	ParseException(const std::string& error) :
		std::runtime_error(error.c_str())
	{
	}
};

class Parser {
public:
	explicit Parser(const std::vector<Token>& tokens);
	std::vector<std::unique_ptr<stmt::Stmt>> Parse();

private:
	std::unique_ptr<expr::Expr> Expression();
	std::unique_ptr<expr::Expr> Comma();
	std::unique_ptr<expr::Expr> Assignment();
	std::unique_ptr<expr::Expr> Equality();
	std::unique_ptr<expr::Expr> Comparison();
	std::unique_ptr<expr::Expr> Term();
	std::unique_ptr<expr::Expr> Factor();
	std::unique_ptr<expr::Expr> Unary();
	std::unique_ptr<expr::Expr> Primary();

	std::unique_ptr<stmt::Stmt> Statement();
	std::unique_ptr<stmt::Stmt> Declaration();
	std::unique_ptr<stmt::Stmt> VarDeclaration();
	std::unique_ptr<stmt::Stmt> PrintStatement();
	std::unique_ptr<stmt::Stmt> ExpressionStatement();

	std::vector<std::unique_ptr<stmt::Stmt>> Block();

	const Token& CurrentToken() const;
	const Token& PreviousToken() const;
	bool IsAtEnd() const;
	const Token& Advance();
	const Token& Consume(TokenType type, const std::string& message);
	ParseException Error(const Token& token, const std::string& message);
	void Synchronize();

	template<IsTokenType ...Args>
	bool Match(Args ...args);

	std::vector<Token> m_Tokens;
	std::size_t m_Current;
};

template<IsTokenType ...Args>
inline bool Parser::Match(Args ...args)
{
	if (((CurrentToken().type == args) || ...)) {
		Advance();
		return true;
	}
	return false;
}

