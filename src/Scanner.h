#pragma once
#include <vector>
#include <string>
#include "Token.h"

class Scanner {
public:
	explicit Scanner(std::string sourceCode);
	std::vector<Token> ScanTokens();

private:
	void ScanToken();
	bool IsAtEnd() const;
	char Advance();
	char Peek() const;
	char PeekNext() const;
	void AddToken(TokenType type);
	void AddToken(TokenType type, const std::any& literal);
	bool Match(char expected);
	void ConsumeString();
	void ConsumeDigit();
	void ConsumeIdentifier();
	void ConsumeBlockComment();

	std::vector<Token> m_Tokens;
	std::string m_SourceCode;
	std::size_t m_Start;
	std::size_t m_Current;
	std::size_t m_Line;
};