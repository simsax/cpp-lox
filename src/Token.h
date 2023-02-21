#pragma once
#include <cstdint>
#include <iostream>
#include <variant>

enum class TokenType : uint8_t {
	// Single-character tokens
	LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

	// One or two character tokens
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,

	// Literals
	IDENTIFIER, STRING, NUMBER,

	// Keywords
	AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

	END
};

struct Token {
	inline Token(TokenType type, std::string lexeme, std::variant<std::monostate, double, std::string> literal, int line) :
		type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line)
	{ }

	//inline friend std::ostream& operator<<(std::ostream& os, const Token& token) {
	//	std::visit([&os, &token](const auto& literalVal) {
	//		os << "Type: " << static_cast<int>(token.type) << " Lexeme: " <<
	//		token.lexeme << " Value: " << literalVal; },
	//		token.literal);
	//	return os;
	//}

	TokenType type;
	std::string lexeme;
	std::variant<std::monostate, double, std::string> literal;
	int line;
};