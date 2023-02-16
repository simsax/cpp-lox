#include <iostream>
#include "Lox.h"


int main(int argc, char** argv) {
	Lox* lox = new Lox();
	if (argc > 2) {
		std::cerr << "Too many arguments. Usage: cpplox [script].\n";
		delete lox;
		std::exit(EX_USAGE);
	}
	else if (argc == 2) {
		lox->RunFile(argv[1]);
	}
	else {
		lox->RunPrompt();
	}
	delete lox;
	return 0;
}