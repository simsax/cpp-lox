#pragma once
#include <vector>
#include <string>
#include "Token.h"

class Scanner {
public:
	Scanner(std::string sourceCode);
	std::vector<Token> ScanTokens();

private:
	void ScanToken();
	bool IsAtEnd();
	char Advance();
	char Peek();
	char PeekNext();
	void AddToken(TokenType type);
	void AddToken(TokenType type, const std::variant<std::monostate, double, std::string>& literal);
	bool Match(char expected);
	void ConsumeString();
	void ConsumeDigit();
	void ConsumeIdentifier();
	void ConsumeBlockComment();

	std::vector<Token> m_Tokens;
	std::string m_SourceCode;
	uint32_t m_Start;
	uint32_t m_Current;
	uint32_t m_Line;
};