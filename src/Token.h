#pragma once
#include <cstdint>
#include <iostream>
#include <any>

enum class TokenType : uint8_t {
	// Single-character tokens
	LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

	// One or two character tokens
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,
	PLUS_EQUAL, MINUS_EQUAL,
	STAR_EQUAL, SLASH_EQUAL,

	// Literals
	IDENTIFIER, STRING, NUMBER,

	// Keywords
	AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
	BREAK, CONTINUE,

	END
};

struct Token {
	inline Token(TokenType type, std::string lexeme, std::any literal, std::size_t line) :
		type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line)
	{ }

	TokenType type;
	std::string lexeme;
	std::any literal;
	std::size_t line;
};