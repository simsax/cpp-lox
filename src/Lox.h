#pragma once

#include <fstream>
#include <sstream>
#include <vector>

namespace Lox {
	void Run(const std::string& sourceCode);
	void RunFile(const char* fileName);
	void RunPrompt();
	void Error(int line, const std::string& message);
};
