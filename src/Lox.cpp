#include <iostream>
#include "Lox.h"
#include "Scanner.h"
#include "Token.h"

#define EX_USAGE 64
#define EX_DATAERR 65

namespace Lox {

	namespace {
		bool m_HadError = false;
		void Report(int line, const std::string& where, const std::string& message) {
			std::cerr << "[line " << line << "] Error" << where << "" << ": " << message << "\n";
			m_HadError = true;
		}
	}

	void Error(int line, const std::string& message) {
		Report(line, "", message);
	}

	void Run(const std::string& sourceCode) {
		Scanner scanner(sourceCode);
		std::vector<Token> tokens = scanner.ScanTokens();

		for (const auto& token : tokens) {
			std::cout << token << "\n";
		}
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
			if (m_HadError)
				std::exit(EX_DATAERR);
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
