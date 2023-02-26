#pragma once
#include <vector>
#include <concepts>
#include "Token.h"
#include "Expr.h"

template<typename T>
concept IsTokenType = std::is_same<T, TokenType>::value;

class Parser {
public:
	Parser(const std::vector<Token>& tokens);

private:
	Expr* Expression();
	Expr* Equality();
	Expr* Comparison();
	Expr* Term();
	Expr* Factor();
	Expr* Unary();
	Expr* Primary();

	Token& CurrentToken();
	Token& PreviousToken();
	bool IsAtEnd();
	Token& Advance();

	template<IsTokenType ...Args>
	bool Match(Args ...args);

	std::vector<Token> m_Tokens;
	int m_Current;
};

template<IsTokenType ...Args>
inline bool Parser::Match(Args ...args)
{
	if ((CurrentToken().type == args || ...)) {
		Advance();
		return true;
	}
	return false;
}
