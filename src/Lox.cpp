#include <iostream>
#include "Lox.h"
#include "Scanner.h"
#include "Parser.h"
#include "Interpreter.h"

#define EX_USAGE 64
#define EX_DATAERR 65
#define EX_RUNNINGERR 70

namespace Lox {

	namespace {
		bool m_HadError = false;
		bool m_HadRuntimeError = false;
		static Interpreter m_Interpreter;

		void Report(std::size_t line, const std::string& where, const std::string& message) {
			std::cerr << "[line " << line << "] Error" << where << "" << ": " << message << "\n";
			m_HadError = true;
		}
	}

	void Error(std::size_t line, const std::string& message) {
		Report(line, "", message);
	}

	void Error(const Token& token, const std::string& message)
	{
		if (token.type == TokenType::END)
			Report(token.line, " at end", message);
		else
			Report(token.line, " at '" + token.lexeme + "'", message);
	}

	void RuntimeError(const Token& token, const std::string& message)
	{
		std::cout << message << "\n[line " << token.line << "]\n";
		m_HadRuntimeError = true;
	}

	void Run(const std::string& sourceCode) {
		Scanner scanner(sourceCode);
		std::vector<Token> tokens = scanner.ScanTokens();
		Parser parser = Parser(tokens);
		std::vector<std::unique_ptr<stmt::Stmt>> statements = parser.Parse();
		if (m_HadError)
			return;
		m_Interpreter.Interpret(statements);
	}

	void RunFile(const char* fileName) {
		std::ifstream file(fileName, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			std::cerr << "Failed to open file: " << fileName << "\n";
			std::exit(EX_USAGE);
		}
		else {
			auto fileSize = file.tellg();
			std::string text(fileSize, '\0');
			file.seekg(0);
			file.read(&text[0], fileSize);
			file.close();
			Run(text);
			if (m_HadError) std::exit(EX_DATAERR);
			if (m_HadRuntimeError) std::exit(EX_RUNNINGERR);
		}
	}

	void RunPrompt() {
		std::string line;
		std::cout << "> ";
		while (std::getline(std::cin, line)) {
			if (!line.empty())
				Run(line);
			std::cout << "> ";
			m_HadError = false;
		}
	}
}
