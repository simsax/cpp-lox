#include <iostream>
#include "Lox.h"
#include "AstPrinter.h"
#include "Expr.h"

#define EX_USAGE 64

int main(int argc, char** argv) {
	if (argc > 2) {
		std::cerr << "Too many arguments. Usage: cpplox [script].\n";
		std::exit(EX_USAGE);
	}
	else if (argc == 2) {
		Lox::RunFile(argv[1]);
	}
	else {
		Lox::RunPrompt();
	}
	return 0;
}