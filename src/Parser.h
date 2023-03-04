#pragma once
#include <vector>
#include <concepts>
#include <exception>
#include <stdexcept>
#include "Token.h"
#include "Expr.h"

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
	Parser(const std::vector<Token>& tokens);
	std::unique_ptr<Expr> Parse();

private:
	std::unique_ptr<Expr> Expression();
	std::unique_ptr<Expr> Equality();
	std::unique_ptr<Expr> Comparison();
	std::unique_ptr<Expr> Term();
	std::unique_ptr<Expr> Factor();
	std::unique_ptr<Expr> Unary();
	std::unique_ptr<Expr> Primary();

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

