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
declaration	   → varDecl | statement | funDecl ;
funDecl		   → "fun" function ;
function	   → IDENTIFIER "(" parameters? ")" block ;
parameters	   → IDENTIFIER ("," IDENTIFIER)* ;
varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
statement      → exprStmt | printStmt | blockStmt | ifStmt | whileStmt | forStmt |
				breakStmt | continueStmt | returnStmt;
forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
				 expression? ";"
				 expression? ")" statement ;
whileStmt      → "while" "(" expression ")" statement ;
ifStmt		   → "if" "(" expression ")" statement ( "else" statement )? ;
exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
blockStmt      → "{" declaration* "}" ;
breakStmt	   → "break" ";" ;
continueStmt   → "continue" ";" ;
block          → "{" declaration* "}" ;
returnStmt	   → "return" expression? ";" ;

// Expressions
comma		   → expression ( "," expression )* ;
expression     → assignment | opr_assignment ;
assignment     → IDENTIFIER "=" assignment | ternary ;
opr_assignment → IDENTIFIER ( "+=" | "-=" | "*=" | "/=" ) ternary ;
ternary		   → logic_or "?" comma ":" ternary | logic_or ;
logic_or       → logic_and ( "or" logic_and )* ;
logic_and      → equality ( "and" equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary | call ;
call           → primary ( "(" arguments? ")" )* ;
arguments	   → expression ("," expression)* ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
			   | "(" expression ")" | IDENTIFIER ;
*/

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
	std::unique_ptr<expr::Expr> OprAssignment();
	std::unique_ptr<expr::Expr> Or();
	std::unique_ptr<expr::Expr> And();
	std::unique_ptr<expr::Expr> Equality();
	std::unique_ptr<expr::Expr> Comparison();
	std::unique_ptr<expr::Expr> Term();
	std::unique_ptr<expr::Expr> Factor();
	std::unique_ptr<expr::Expr> Unary();
	std::unique_ptr<expr::Expr> Primary();
	std::unique_ptr<expr::Expr> Ternary();
	std::unique_ptr<expr::Expr> Call();
	std::unique_ptr<expr::Expr> FinishCall(std::unique_ptr<expr::Expr> expr);

	std::unique_ptr<stmt::Stmt> Statement();
	std::unique_ptr<stmt::Stmt> Declaration();
	std::unique_ptr<stmt::Stmt> VarDeclaration();
	std::unique_ptr<stmt::Stmt> PrintStatement();
	std::unique_ptr<stmt::Stmt> ExpressionStatement();
	std::unique_ptr<stmt::Stmt> IfStatement();
	std::unique_ptr<stmt::Stmt> WhileStatement();
	std::unique_ptr<stmt::Stmt> ForStatement();
	std::unique_ptr<stmt::Stmt> Function(const std::string& kind);
	std::unique_ptr<stmt::Stmt> ReturnStatement();
	std::unique_ptr<stmt::Stmt> JumpStatement();

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
	bool m_InsideLoop;
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

