#include "Parser.h"
#include "Lox.h"

Parser::Parser(const std::vector<Token>& tokens) :
	m_Tokens(tokens), m_Current(0)
{
}

std::vector<std::unique_ptr<stmt::Stmt>> Parser::Parse()
{
	std::vector<std::unique_ptr<stmt::Stmt>> statements; // vector of trees
	while (!IsAtEnd()) {
		statements.emplace_back(Declaration());
	}
	return statements;
}

std::unique_ptr<expr::Expr> Parser::Expression() {
	return Assignment();
}

std::unique_ptr<expr::Expr> Parser::Assignment()
{
	std::unique_ptr<expr::Expr> expr = Or();
	if (Match(TokenType::EQUAL)) {
		const Token& equals = PreviousToken();
		std::unique_ptr<expr::Expr> value = Assignment();
		if (auto var = dynamic_cast<expr::Variable*>(expr.get())) {
			return std::make_unique<expr::Assign>(var->m_Name, std::move(value));
		}
		else {
			Error(equals, "Invalid assignment target.");
		}
	}

	return expr;
}

std::unique_ptr<expr::Expr> Parser::Or()
{
	std::unique_ptr<expr::Expr> expr = And();
	while (Match(TokenType::OR)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = And();
		expr = std::make_unique<expr::Logical>(std::move(expr), opr, std::move(right));
	}
	return expr;
}

std::unique_ptr<expr::Expr> Parser::And()
{
	std::unique_ptr<expr::Expr> expr = Equality();
	while (Match(TokenType::AND)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = Equality();
		expr = std::make_unique<expr::Logical>(std::move(expr), opr, std::move(right));
	}
	return expr;
}

std::unique_ptr<expr::Expr> Parser::Equality() {
	std::unique_ptr<expr::Expr> expr = Comparison();

	while (Match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = Comparison();
		expr = std::make_unique<expr::Binary>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<expr::Expr> Parser::Comparison()
{
	std::unique_ptr<expr::Expr> expr = Term();

	while (Match(TokenType::GREATER, TokenType::GREATER_EQUAL,
		TokenType::LESS, TokenType::LESS_EQUAL)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = Term();
		expr = std::make_unique<expr::Binary>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<expr::Expr> Parser::Term()
{
	std::unique_ptr<expr::Expr> expr = Factor();

	while (Match(TokenType::MINUS, TokenType::PLUS)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = Factor();
		expr = std::make_unique<expr::Binary>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<expr::Expr> Parser::Factor()
{
	std::unique_ptr<expr::Expr> expr = Unary();

	while (Match(TokenType::STAR, TokenType::SLASH)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = Unary();
		expr = std::make_unique<expr::Binary>(std::move(expr), opr, std::move(right));
	}

	return expr;
}

std::unique_ptr<expr::Expr> Parser::Unary()
{
	while (Match(TokenType::BANG, TokenType::MINUS)) {
		const Token& opr = PreviousToken();
		return std::make_unique<expr::Unary>(opr, Unary());
	}

	return Call();
}
std::unique_ptr<expr::Expr> Parser::Primary() {
	if (Match(TokenType::FALSE))
		return std::make_unique<expr::Literal>(false);
	if (Match(TokenType::TRUE))
		return std::make_unique<expr::Literal>(true);
	if (Match(TokenType::NIL))
		return std::make_unique<expr::Literal>(nullptr);
	if (Match(TokenType::NUMBER, TokenType::STRING))
		return std::make_unique<expr::Literal>(PreviousToken().literal);
	if (Match(TokenType::IDENTIFIER))
		return std::make_unique<expr::Variable>(PreviousToken());
	if (Match(TokenType::LEFT_PAREN)) {
		std::unique_ptr<expr::Expr> expr = Expression();
		Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
		return std::make_unique<expr::Grouping>(std::move(expr));
	}
	throw Error(CurrentToken(), "Expect expression.");
}

std::unique_ptr<expr::Expr> Parser::Call()
{
	std::unique_ptr<expr::Expr> expr = Primary();
	while (true) {
		if (Match(TokenType::LEFT_PAREN)) {
			expr = FinishCall(std::move(expr));
		}
		else {
			break;
		}
	}
	return expr;
}

std::unique_ptr<expr::Expr> Parser::FinishCall(std::unique_ptr<expr::Expr> expr)
{
	std::vector<std::unique_ptr<expr::Expr>> arguments;

	if (!(CurrentToken().type == TokenType::RIGHT_PAREN)) {
		do {
			if (arguments.size() >= 255) {
				Error(CurrentToken(), "Can't have more than 255 arguments.");
			}
			arguments.emplace_back(Expression());
		} while (Match(TokenType::COMMA));
	}
	const Token& paren = Consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

	return std::make_unique<expr::Call>(std::move(expr), paren, std::move(arguments));
}

std::unique_ptr<stmt::Stmt> Parser::Statement()
{
	if (Match(TokenType::PRINT))
		return PrintStatement();
	if (Match(TokenType::LEFT_BRACE))
		return std::make_unique<stmt::Block>(Block());
	if (Match(TokenType::IF))
		return IfStatement();
	if (Match(TokenType::WHILE))
		return WhileStatement();
	if (Match(TokenType::FOR))
		return ForStatement();
	return ExpressionStatement();
}

std::vector<std::unique_ptr<stmt::Stmt>> Parser::Block()
{
	std::vector<std::unique_ptr<stmt::Stmt>> statements;
	while (!IsAtEnd() && CurrentToken().type != TokenType::RIGHT_BRACE) {
		statements.emplace_back(Declaration());
	}
	Consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
	return statements;
}

std::unique_ptr<stmt::Stmt> Parser::Declaration()
{
	try {
		if (Match(TokenType::VAR))
			return VarDeclaration();
		return Statement();
	}
	catch (const ParseException&) {
		Synchronize();
		return std::unique_ptr<stmt::Stmt>{};
	}
}

std::unique_ptr<stmt::Stmt> Parser::VarDeclaration()
{
	const Token& name = Consume(TokenType::IDENTIFIER, "Expect variable name.");
	std::unique_ptr<expr::Expr> initializer;
	if (Match(TokenType::EQUAL))
		initializer = Expression();
	Consume(TokenType::SEMICOLON, "Expect ';' after value.");
	return std::make_unique<stmt::Var>(name, std::move(initializer));
}

std::unique_ptr<stmt::Stmt> Parser::PrintStatement()
{
	std::unique_ptr<expr::Expr> value = Expression();
	Consume(TokenType::SEMICOLON, "Expect ';' after value.");
	return std::make_unique<stmt::Print>(std::move(value));
}

std::unique_ptr<stmt::Stmt> Parser::ExpressionStatement()
{
	std::unique_ptr<expr::Expr> expr = Expression();
	Consume(TokenType::SEMICOLON, "Expect ';' after expression.");
	return std::make_unique<stmt::Expression>(std::move(expr));
}

std::unique_ptr<stmt::Stmt> Parser::IfStatement()
{
	Consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
	std::unique_ptr<expr::Expr> condition = Expression();
	Consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
	std::unique_ptr<stmt::Stmt> thenBranch = Statement();
	std::unique_ptr<stmt::Stmt> elseBranch = nullptr;
	if (Match(TokenType::ELSE))
		elseBranch = Statement();
	return std::make_unique<stmt::If>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<stmt::Stmt> Parser::WhileStatement()
{
	Consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
	std::unique_ptr<expr::Expr> condition = Expression();
	Consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
	std::unique_ptr<stmt::Stmt> statement = Statement();
	return std::make_unique<stmt::While>(std::move(condition), std::move(statement));
}

std::unique_ptr<stmt::Stmt> Parser::ForStatement()
{
	Consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
	std::unique_ptr<stmt::Stmt> initializer;
	if (Match(TokenType::SEMICOLON))
		initializer = nullptr;
	else if (Match(TokenType::VAR))
		initializer = VarDeclaration();
	else
		initializer = ExpressionStatement();
	std::unique_ptr<expr::Expr> condition = nullptr;
	if (CurrentToken().type != TokenType::SEMICOLON)
		condition = Expression();
	Consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
	std::unique_ptr<expr::Expr> increment = nullptr;
	if (CurrentToken().type != TokenType::RIGHT_PAREN)
		increment = Expression();
	Consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
	std::unique_ptr<stmt::Stmt> body = Statement();

	if (increment.get() != nullptr) {
		// https://tristanbrindle.com/posts/beware-copies-initializer-list
		std::vector<std::unique_ptr<stmt::Stmt>> statements;
		statements.reserve(2);
		statements.emplace_back(std::move(body));
		statements.emplace_back(std::make_unique<stmt::Expression>(std::move(increment)));
		body = std::make_unique<stmt::Block>(std::move(statements));
	}

	if (condition.get() == nullptr)
		condition = std::make_unique<expr::Literal>(true);
	body = std::make_unique<stmt::While>(std::move(condition), std::move(body));

	if (initializer.get() != nullptr) {
		std::vector<std::unique_ptr<stmt::Stmt>> statements;
		statements.reserve(2);
		statements.emplace_back(std::move(initializer));
		statements.emplace_back(std::move(body));
		body = std::make_unique<stmt::Block>(std::move(statements));
	}

	return std::move(body);
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
