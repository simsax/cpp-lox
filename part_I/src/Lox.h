#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include "Token.h"

namespace Lox {
	void Run(const std::string& sourceCode);
	void RunFile(const char* fileName);
	void RunPrompt();
	void Error(std::size_t line, const std::string& message);
	void Error(const Token& token, const std::string& message);
	void RuntimeError(const Token& token, const std::string& message);
};
