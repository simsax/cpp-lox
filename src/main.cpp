#include <iostream>
#include "Lox.h"
#include "RPNPrinter.h"
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

	// leaking memory
	Expr* expression = new Binary(
		new Binary(
			new Literal(1),
			Token(TokenType::PLUS, "+", std::any{}, 1),
			new Literal(2)
		),
		Token(TokenType::STAR, "*", std::any{}, 1),
		new Binary(
			new Literal(4),
			Token(TokenType::MINUS, "-", std::any{}, 1),
			new Literal(3)
		)
	);

	RPNPrinter rpnPrinter;
	std::cout << rpnPrinter.Print(expression);
	return 0;
}
