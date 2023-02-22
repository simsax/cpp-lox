#include <iostream>
#include "Lox.h"
#include "AstPrinter.h"
#include "Expr.h"

#define EX_USAGE 64

//int main(int argc, char** argv) {
int main() {
	//if (argc > 2) {
	//	std::cerr << "Too many arguments. Usage: cpplox [script].\n";
	//	std::exit(EX_USAGE);
	//}
	//else if (argc == 2) {
	//	Lox::RunFile(argv[1]);
	//}
	//else {
	//	Lox::RunPrompt();
	//}
	//return 0;

	Expr* expression = new Binary(
		new Unary(
			Token(TokenType::MINUS, "-", std::monostate(), 1),
			new Literal(123)),
		Token(TokenType::STAR, "*", std::monostate(), 1),
		new Grouping(new Literal(45.67)));

	AstPrinter astPrinter;
	std::cout << astPrinter.Print(expression);
	return 0;
}