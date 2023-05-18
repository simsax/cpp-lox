#include "Parser.h"
#include "Lox.h"
#include <memory>

Parser::Parser(const std::vector<Token>& tokens)
	: m_Tokens(tokens)
	, m_Current(0)
	, m_InsideLoop(false)
{
}

std::vector<std::unique_ptr<stmt::Stmt>> Parser::Parse()
{
	std::vector<std::unique_ptr<stmt::Stmt>> statements;
	while (!IsAtEnd()) {
		statements.emplace_back(Declaration());
	}
	return statements;
}

std::unique_ptr<expr::Expr> Parser::Expression()
{
	if (Match(TokenType::FUN)) {
		return AnonFunDecl();
	}
	return Assignment();
}

std::unique_ptr<expr::Expr> Parser::Assignment()
{
	std::unique_ptr<expr::Expr> expr = Ternary();
	if (Match(TokenType::EQUAL)) {
		const Token& equals = PreviousToken();
		std::unique_ptr<expr::Expr> value = Assignment();
		if (auto var = dynamic_cast<expr::Variable*>(expr.get())) {
			return std::make_unique<expr::Assign>(var->m_Name, std::move(value));
		}
		else if (auto getExpr = dynamic_cast<expr::Get*>(expr.get())) {
			return std::make_unique<expr::Set>(
				std::move(getExpr->m_Object), getExpr->m_Name, std::move(value));
		}
		else {
			Error(equals, "Invalid assignment target.");
		}
	}
	if (Match(TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::STAR_EQUAL,
		TokenType::SLASH_EQUAL)) {
		const Token& oprEquals = PreviousToken();
		std::unique_ptr<expr::Expr> value = Ternary();
		if (auto var = dynamic_cast<expr::Variable*>(expr.get())) {
			return std::make_unique<expr::OprAssign>(var->m_Name, oprEquals, std::move(value));
		}
		else if (auto getExpr = dynamic_cast<expr::Get*>(expr.get())) {
			return std::make_unique<expr::OprSet>(
				std::move(getExpr->m_Object), getExpr->m_Name, oprEquals, std::move(value));
		}
		else {
			Error(oprEquals, "Invalid assignment target.");
		}
	}
	return expr;
}

std::unique_ptr<expr::Expr> Parser::Comma()
{
	std::unique_ptr<expr::Expr> expr = Expression();

	while (Match(TokenType::COMMA)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> right = Expression();
		expr = std::make_unique<expr::Binary>(std::move(expr), opr, std::move(right));
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

std::unique_ptr<expr::Expr> Parser::Equality()
{
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

	while (Match(
		TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL)) {
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
std::unique_ptr<expr::Expr> Parser::Primary()
{
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
	if (Match(TokenType::THIS)) {
		return std::make_unique<expr::This>(PreviousToken());
	}
	if (Match(TokenType::SUPER)) {
		const Token& keyword = PreviousToken();
		Consume(TokenType::DOT, "Expect '.' after 'super'.");
		const Token& method = Consume(TokenType::IDENTIFIER, "Expect superclass method name.");
		return std::make_unique<expr::Super>(keyword, method);
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
		else if (Match(TokenType::DOT)) {
			const Token& name = Consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
			expr = std::make_unique<expr::Get>(std::move(expr), name);
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

std::unique_ptr<expr::Expr> Parser::AnonFunDecl()
{
	Consume(TokenType::LEFT_PAREN, "Expect '(' after 'fun'.");
	std::vector<Token> params;
	if (CurrentToken().type != TokenType::RIGHT_PAREN) {
		do {
			if (params.size() >= 255) {
				Error(CurrentToken(), "Can't have more than 255 parameters.");
			}
			Token param = Consume(TokenType::IDENTIFIER, "Expect parameter name.");
			params.emplace_back(std::move(param));
		} while (Match(TokenType::COMMA));
	}
	Consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
	Consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
	std::vector<std::unique_ptr<stmt::Stmt>> body = Block();
	return std::make_unique<expr::AnonFunction>(std::move(params), std::move(body));
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
	if (Match(TokenType::RETURN))
		return ReturnStatement();
	if (Match(TokenType::BREAK, TokenType::CONTINUE))
		return JumpStatement();
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
		if (Match(TokenType::FUN))
			return Function("Function");
		if (Match(TokenType::CLASS))
			return ClassDeclaration();
		return Statement();
	}
	catch (const ParseException&) {
		Synchronize();
		return std::unique_ptr<stmt::Stmt> {};
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
	return std::make_unique<stmt::If>(
		std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<expr::Expr> Parser::Ternary()
{
	std::unique_ptr<expr::Expr> condition = Or();
	if (Match(TokenType::QUESTION_MARK)) {
		const Token& opr = PreviousToken();
		std::unique_ptr<expr::Expr> thenExpr = Comma();
		if (Match(TokenType::COLON)) {
			std::unique_ptr<expr::Expr> elseExpr = Ternary();
			return std::make_unique<expr::Ternary>(
				std::move(condition), std::move(thenExpr), std::move(elseExpr));
		}
		else {
			Error(opr, "Expect ':' after '?'.");
		}
	}

	return condition;
}

std::unique_ptr<stmt::Stmt> Parser::WhileStatement()
{
	m_InsideLoop = true;
	Consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
	std::unique_ptr<expr::Expr> condition = Expression();
	Consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
	std::unique_ptr<stmt::Stmt> statement = Statement();
	m_InsideLoop = false;
	return std::make_unique<stmt::While>(std::move(condition), std::move(statement));
}

std::unique_ptr<stmt::Stmt> Parser::ForStatement()
{
	m_InsideLoop = true;
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

	if (condition.get() == nullptr)
		condition = std::make_unique<expr::Literal>(true);

	m_InsideLoop = false;
	return std::make_unique<stmt::For>(
		std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

std::unique_ptr<stmt::Function> Parser::Function(const std::string& kind)
{
	const Token& name = Consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
	if (Match(TokenType::LEFT_BRACE)) {
		return Getter(name);
	}
	Consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
	std::vector<Token> params;
	if (CurrentToken().type != TokenType::RIGHT_PAREN) {
		do {
			if (params.size() >= 255) {
				Error(CurrentToken(), "Can't have more than 255 parameters.");
			}
			Token param = Consume(TokenType::IDENTIFIER, "Expect parameter name.");
			params.emplace_back(std::move(param));
		} while (Match(TokenType::COMMA));
	}
	Consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
	Consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
	std::vector<std::unique_ptr<stmt::Stmt>> body = Block();
	return std::make_unique<stmt::Function>(name, std::move(params), std::move(body), false);
}

std::unique_ptr<stmt::Stmt> Parser::ReturnStatement()
{
	const Token& keyword = PreviousToken();
	std::unique_ptr<expr::Expr> value = nullptr;
	if (CurrentToken().type != TokenType::SEMICOLON)
		value = Expression();
	Consume(TokenType::SEMICOLON, "Expect ';' after return value.");
	return std::make_unique<stmt::Return>(keyword, std::move(value));
}

std::unique_ptr<stmt::Function> Parser::Getter(const Token& name)
{
	std::vector<std::unique_ptr<stmt::Stmt>> body = Block();
	return std::make_unique<stmt::Function>(name, std::vector<Token> {}, std::move(body), true);
}

std::unique_ptr<stmt::Stmt> Parser::ClassDeclaration()
{
	const Token& name = Consume(TokenType::IDENTIFIER, "Expect class name.");
	std::unique_ptr<expr::Variable> superClass;
	if (Match(TokenType::LESS)) {
		const Token& superClassName = Consume(TokenType::IDENTIFIER, "Expect superclass name.");
		superClass = std::make_unique<expr::Variable>(superClassName);
	}
	Consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");
	stmt::MethodVec methods;
	stmt::MethodVec classMethods;
	while (CurrentToken().type != TokenType::RIGHT_BRACE && !IsAtEnd()) {
		if (Match(TokenType::CLASS)) {
			classMethods.emplace_back(Function("class method"));
		}
		else {
			methods.emplace_back(Function("method"));
		}
	}
	Consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
	return std::make_unique<stmt::Class>(name, std::move(superClass), std::move(methods), std::move(classMethods));
}
std::unique_ptr<stmt::Stmt> Parser::JumpStatement()
{
	const Token& opr = PreviousToken();
	if (!m_InsideLoop)
		throw Error(opr, "'" + opr.lexeme + "' cannot be used outside of a loop.");
	Consume(TokenType::SEMICOLON, "Expect ';' after '" + opr.lexeme + "'.");
	return std::make_unique<stmt::Jump>(opr);
}

const Token& Parser::CurrentToken() const { return m_Tokens[m_Current]; }

const Token& Parser::PreviousToken() const { return m_Tokens[m_Current - 1]; }

bool Parser::IsAtEnd() const { return CurrentToken().type == TokenType::END; }

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

		switch (CurrentToken().type) {
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
