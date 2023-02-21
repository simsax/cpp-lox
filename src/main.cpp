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
	Expr<std::string>* expression = new Binary<std::string>(
		new Unary<std::string>(
			Token(TokenType::MINUS, "-", std::monostate(), 1),
			new Literal<std::string>(123.0)),
		Token(TokenType::STAR, "*", std::monostate(), 1),
		new Grouping<std::string>(new Literal<std::string>(45.67)));

	AstPrinter astPrinter;
	std::cout << astPrinter.Print(expression);
	return 0;
}