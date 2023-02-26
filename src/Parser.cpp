#include "Parser.h"

Parser::Parser(const std::vector<Token>& tokens) :
	m_Tokens(tokens), m_Current(0)
{
}

Expr* Parser::Expression() {
	return Equality();
}


// leaking like crazy: remember to fix this (or just use smart pointers)
// or just use references to stack allocated objects
Expr* Parser::Equality() {
	Expr* expr = Comparison();

	while (Match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL)) {
		Token& opr = PreviousToken();
		Expr* right = Comparison();
		expr = new BinaryExpr(expr, opr, right);
	}

	return expr;
}

Expr* Parser::Comparison()
{
	Expr* expr = Term();

	while (Match(TokenType::GREATER, TokenType::GREATER_EQUAL,
		TokenType::LESS, TokenType::LESS_EQUAL)) {
		Token& opr = PreviousToken();
		Expr* right = Term();
		expr = new BinaryExpr(expr, opr, right);
	}

	return expr;
}

Expr* Parser::Term()
{
	Expr* expr = Factor();

	while (Match(TokenType::MINUS, TokenType::PLUS)) {
		Token& opr = PreviousToken();
		Expr* right = Factor();
		expr = new BinaryExpr(expr, opr, right);
	}

	return expr;
}

Expr* Parser::Factor()
{
	Expr* expr = Unary();

	while (Match(TokenType::STAR, TokenType::SLASH)) {
		Token& opr = PreviousToken();
		Expr* right = Unary();
		expr = new BinaryExpr(expr, opr, right);
	}

	return expr;
}

Expr* Parser::Unary()
{
	while (Match(TokenType::BANG, TokenType::MINUS)) {
		Token& opr = PreviousToken();
		return new UnaryExpr(opr, Unary());
	}

	return Primary();
}

Expr* Parser::Primary() {
	if (Match(TokenType::FALSE))
		return new LiteralExpr(false);
	if (Match(TokenType::TRUE))
		return new LiteralExpr(true);
	if (Match(TokenType::NIL))
		return new LiteralExpr(nullptr);
	if (Match(TokenType::NUMBER, TokenType::STRING))
		return new LiteralExpr(PreviousToken().literal);
	if (Match(TokenType::LEFT_PAREN)) {
		Expr* expr = Expression();
		Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
		return new GroupingExpr(expr);
	}
}

Token& Parser::CurrentToken()
{
	return m_Tokens[m_Current];
}

Token& Parser::PreviousToken()
{
	return m_Tokens[m_Current - 1];
}

bool Parser::IsAtEnd()
{
	return CurrentToken().type == TokenType::END;
}

Token& Parser::Advance()
{
	if (!IsAtEnd())
		m_Current++;
	return PreviousToken();
}
