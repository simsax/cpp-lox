#include "Scanner.h"
#include "Lox.h"
#include <unordered_map>

static bool IsDigit(char c) {
	return c >= '0' && c <= '9';
}

static bool IsAlpha(char c) {
	return  (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

static bool IsAlphaNumeric(char c) {
	return IsAlpha(c) || IsDigit(c);
}

static const std::unordered_map<std::string, TokenType> s_ReservedKeywords = {
	{"and",    TokenType::AND},
	{"class",  TokenType::CLASS},
	{"else",   TokenType::ELSE},
	{"false",  TokenType::FALSE},
	{"for",    TokenType::FOR},
	{"fun",    TokenType::FUN},
	{"if",     TokenType::IF},
	{"nil",    TokenType::NIL},
	{"or",     TokenType::OR},
	{"print",  TokenType::PRINT},
	{"return", TokenType::RETURN},
	{"super",  TokenType::SUPER},
	{"this",   TokenType::THIS},
	{"true",   TokenType::TRUE},
	{"var",    TokenType::VAR},
	{"while",  TokenType::WHILE}
};

Scanner::Scanner(std::string sourceCode) :
	m_SourceCode(std::move(sourceCode)),
	m_Start(0),
	m_Current(0),
	m_Line(1)
{
}

std::vector<Token> Scanner::ScanTokens()
{
	while (!IsAtEnd()) {
		m_Start = m_Current;
		ScanToken();
	}

	m_Tokens.emplace_back(TokenType::END, "", "", m_Line);
	return m_Tokens;
}

void Scanner::ScanToken() {
	char c = Advance();
	switch (c)
	{
	case '(':
		AddToken(TokenType::LEFT_PAREN);
		break;
	case ')':
		AddToken(TokenType::RIGHT_PAREN);
		break;
	case '{':
		AddToken(TokenType::LEFT_BRACE);
		break;
	case '}':
		AddToken(TokenType::RIGHT_BRACE);
		break;
	case ',':
		AddToken(TokenType::COMMA);
		break;
	case '.':
		AddToken(TokenType::DOT);
		break;
	case '-':
		AddToken(TokenType::MINUS);
		break;
	case '+':
		AddToken(TokenType::PLUS);
		break;
	case ';':
		AddToken(TokenType::SEMICOLON);
		break;
	case '*':
		AddToken(TokenType::STAR);
		break;
	case '!':
		AddToken(Match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
		break;
	case '=':
		AddToken(Match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
		break;
	case '<':
		AddToken(Match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
		break;
	case '>':
		AddToken(Match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
		break;
	case '/':
		if (Match('/')) {
			// consume characters until the comment ends (end of line)
			while (Peek() != '\n' && !IsAtEnd())
				Advance();
		}
		else {
			AddToken(TokenType::SLASH);
		}
		break;
	case ' ':
	case '\r':
	case '\t':
		// ignore whitespace
		break;
	case '\n':
		m_Line++;
		break;
	case '"':
		ConsumeString();
		break;
	default:
		if (IsDigit(c)) {
			ConsumeDigit();
		}
		else if (IsAlpha(c)) {
			ConsumeIdentifier();
		}
		else {
			Lox::Error(m_Line, "Unexpected character.");
		}
		break;
	}
}

bool Scanner::IsAtEnd() {
	return m_Current >= m_SourceCode.size();
}

char Scanner::Advance() {
	return m_SourceCode[m_Current++];
}

char Scanner::Peek() {
	if (IsAtEnd())
		return '\0';
	return m_SourceCode[m_Current];
}

char Scanner::PeekNext() {
	if (m_Current + 1 >= m_SourceCode.size())
		return '\0';
	return m_SourceCode[m_Current + 1];
}

void Scanner::AddToken(TokenType type) {
	AddToken(type, std::any{});
}

void Scanner::AddToken(TokenType type, const std::any& literal) {
	std::string text = m_SourceCode.substr(m_Start, m_Current - m_Start);
	m_Tokens.emplace_back(type, text, literal, m_Line);
}

bool Scanner::Match(char expected) {
	if (IsAtEnd())
		return false;
	if (m_SourceCode[m_Current] != expected)
		return false;
	m_Current++;
	return true;
}

void Scanner::ConsumeString() {
	while (Peek() != '"' && !IsAtEnd()) {
		if (Peek() == '\n')
			m_Line++;
		Advance();
	}

	if (IsAtEnd()) {
		Lox::Error(m_Line, "Unterminated string.");
		return;
	}

	// the closing '"'
	Advance();

	// trim surrounding quotes
	std::string literal = m_SourceCode.substr(m_Start + 1, m_Current - m_Start - 2);
	AddToken(TokenType::STRING, literal);
}

void Scanner::ConsumeDigit() {
	while (IsDigit(Peek()))
		Advance();

	// look for fractional part
	if (Peek() == '.' && IsDigit(PeekNext())) {
		Advance();

		while (IsDigit(Peek()))
			Advance();
	}

	double digit = std::stod(m_SourceCode.substr(m_Start, m_Current - m_Start));
	AddToken(TokenType::NUMBER, digit);
}

void Scanner::ConsumeIdentifier() {
	while (IsAlphaNumeric(Peek()))
		Advance();

	std::string keyword = m_SourceCode.substr(m_Start, m_Current - m_Start);
	if (const auto item = s_ReservedKeywords.find(keyword); item != s_ReservedKeywords.end())
		AddToken(item->second);
	else
		AddToken(TokenType::IDENTIFIER);
}
