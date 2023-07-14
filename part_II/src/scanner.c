#include <stdio.h>
#include <string.h>
#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

static Scanner scanner;

void init_scanner(const char* source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool is_at_end() { return *scanner.current == '\0'; }

static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static Token error_token(const char* message)
{
    Token token = {
        .type = TOKEN_ERROR, .start = message, .length = (int)strlen(message), .line = scanner.line
    };
    return token;
}

static Token make_token(TokenType type)
{
    Token token = { .type = type,
        .start = scanner.start,
        .length = (int)(scanner.current - scanner.start),
        .line = scanner.line };
    return token;
}

static char peek() { return *scanner.current; }

static char peek_next()
{
    if (is_at_end())
        return '\0';
    return *(scanner.current + 1);
}

static bool match(char expected)
{
    if (is_at_end())
        return false;
    if (peek() != expected)
        return false;
    scanner.current++;
    return true;
}

static void skip_whitespace()
{
    for (;;) {
        char c = peek();
        switch (c) {
        case ' ':
        case '\t':
        case '\r':
            advance();
            break;
        case '\n':
            advance();
            scanner.line++;
            break;
        case '/':
            if (peek_next() == '/') {
                while (peek() != '\n' && !is_at_end())
                    advance();
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static Token string()
{
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n')
            scanner.line++;
        advance();
    }

    if (is_at_end())
        return error_token("Unterminated string.");

    // closing quote
    advance();
    return make_token(TOKEN_STRING);
}

static TokenType check_keyword(int start, int length, const char* rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length
        && memcmp(scanner.start + start, rest, length) == 0)
        return type;
    return TOKEN_IDENTIFIER;
}

static TokenType identifer_type()
{
    switch (scanner.start[0]) {
    case 'a':
        return check_keyword(1, 2, "nd", TOKEN_AND);
    case 'b':
        return check_keyword(1, 4, "reak", TOKEN_BREAK);
    case 'c':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
            case 'a':
                return check_keyword(2, 2, "se", TOKEN_CASE);
            case 'l':
                return check_keyword(2, 3, "ass", TOKEN_CLASS);
            case 'o':
                if (scanner.current - scanner.start > 2) {
                    switch (scanner.start[2]) {
                    case 'n':
                        if (scanner.current - scanner.start > 3) {
                            switch (scanner.start[3]) {
                            case 's':
                                return check_keyword(4, 1, "t", TOKEN_CONST);
                            case 't':
                                return check_keyword(4, 4, "inue", TOKEN_CONTINUE);
                            }
                        }
                    }
                }
            }
        }
    case 'd':
        return check_keyword(1, 6, "efault", TOKEN_DEFAULT);
    case 'e':
        return check_keyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
            case 'a':
                return check_keyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return check_keyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return check_keyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return check_keyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return check_keyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return check_keyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return check_keyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return check_keyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
            case 'u':
                return check_keyword(2, 3, "per", TOKEN_SUPER);
            case 'w':
                return check_keyword(2, 4, "itch", TOKEN_SWITCH);
            }
        }
    case 't':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
            case 'h':
                return check_keyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return check_keyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return check_keyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier()
{
    while (is_alpha(peek()) || is_digit(peek()))
        advance();
    return make_token(identifer_type());
}

static Token number()
{
    while (is_digit(peek()))
        advance();

    // look for a fractional part
    if (peek() == '.' && is_digit(peek_next())) {
        // consume '.'
        advance();
        while (is_digit(peek()))
            advance();
    }

    return make_token(TOKEN_NUMBER);
}

Token scan_token()
{
    skip_whitespace();
    scanner.start = scanner.current;

    if (is_at_end())
        return make_token(TOKEN_EOF);

    char c = advance();
    if (is_digit(c))
        return number();
    if (is_alpha(c))
        return identifier();

    switch (c) {
    case '(':
        return make_token(TOKEN_LEFT_PAREN);
    case ')':
        return make_token(TOKEN_RIGHT_PAREN);
    case '{':
        return make_token(TOKEN_LEFT_BRACE);
    case '}':
        return make_token(TOKEN_RIGHT_BRACE);
    case ';':
        return make_token(TOKEN_SEMICOLON);
    case ',':
        return make_token(TOKEN_COMMA);
    case '.':
        return make_token(TOKEN_DOT);
    case '-':
        return make_token(TOKEN_MINUS);
    case '+':
        return make_token(TOKEN_PLUS);
    case '/':
        return make_token(TOKEN_SLASH);
    case '*':
        return make_token(TOKEN_STAR);
    case '!':
        return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
        return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
        return string();
    case '?':
        return make_token(TOKEN_QUESTION_MARK);
    case ':':
        return make_token(TOKEN_COLON);
    default:
        break;
    }

    return error_token("Unexpected character.");
}