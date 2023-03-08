#include "Parser.h"
#include "Lox.h"

Parser::Parser(const std::vector<Token>& tokens) :
	m_Tokens(tokens), m_Current(0)
{
}

std::unique_ptr<Expr> Parser::Parse()
{
	try {
		return Expression();
	}
	catch (const ParseException& ex) {
		std::cerr << ex.what() << "\n";
		return std::unique_ptr<Expr>{};
	}
}

std::unique_ptr<Expr> Parser::Expression() {
	return Equality();
}

std::unique_ptr<Expr> Parser::Equality() {
	std::unique_ptr<Expr> expr = Comparison();

	while (Match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<Expr> right = Comparison();
		expr = std::make_unique<BinaryExpr>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::Comparison()
{
	std::unique_ptr<Expr> expr = Term();

	while (Match(TokenType::GREATER, TokenType::GREATER_EQUAL,
		TokenType::LESS, TokenType::LESS_EQUAL)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<Expr> right = Term();
		expr = std::make_unique<BinaryExpr>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::Term()
{
	std::unique_ptr<Expr> expr = Factor();

	while (Match(TokenType::MINUS, TokenType::PLUS)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<Expr> right = Factor();
		expr = std::make_unique<BinaryExpr>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::Factor()
{
	std::unique_ptr<Expr> expr = Unary();

	while (Match(TokenType::STAR, TokenType::SLASH)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<Expr> right = Unary();
		expr = std::make_unique<BinaryExpr>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::Unary()
{
	while (Match(TokenType::BANG, TokenType::MINUS)) {
		const Token& opr = PreviousToken();
		return std::make_unique<UnaryExpr>(opr, Unary());
	}

	return Primary();
}

std::unique_ptr<Expr> Parser::Primary() {
	if (Match(TokenType::FALSE))
		return std::make_unique<LiteralExpr>(false);
	if (Match(TokenType::TRUE))
		return std::make_unique<LiteralExpr>(true);
	if (Match(TokenType::NIL))
		return std::make_unique<LiteralExpr>(nullptr);
	if (Match(TokenType::NUMBER, TokenType::STRING))
		return std::make_unique<LiteralExpr>(PreviousToken().literal);
	if (Match(TokenType::LEFT_PAREN)) {
		std::unique_ptr<Expr> expr = Expression();
		Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
		return std::make_unique<GroupingExpr>(std::move(expr));
	}
	throw Error(CurrentToken(), "Expect expression.");
}

const Token& Parser::CurrentToken() const
{
	return m_Tokens[m_Current];
}

const Token& Parser::PreviousToken() const
{
	return m_Tokens[m_Current - 1];
}

bool Parser::IsAtEnd() const
{
	return CurrentToken().type == TokenType::END;
}

const Token& Parser::Advance()
{
	if (!IsAtEnd())
		m_Current++;
	return PreviousToken();
}

const Token& Parser::Consume(TokenType type, const std::string& message)
{
	if (CurrentToken().type == type)
		return Advance();

	throw Error(CurrentToken(), message);
}

ParseException Parser::Error(const Token& token, const std::string& message)
{
	Lox::Error(token, message);
	return ParseException(message);
}

void Parser::Synchronize()
{
	Advance();

	while (!IsAtEnd()) {
		if (PreviousToken().type == TokenType::SEMICOLON)
			return;

		switch (CurrentToken().type)
		{
		case TokenType::CLASS:
		case TokenType::FUN:
		case TokenType::VAR:
		case TokenType::FOR:
		case TokenType::IF:
		case TokenType::WHILE:
		case TokenType::PRINT:
		case TokenType::RETURN:
			return;
		default:
			break;
		}
		Advance();
	}
}
