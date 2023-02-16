#pragma once

#include <fstream>
#include <sstream>
#include <vector>

#define EX_USAGE 64

// should make this a namespace
class Lox {
public:
	static void Run(const std::string& sourceCode);
	static void RunFile(const char* fileName);
	static void RunPrompt();
	static void Error(int line, const std::string& message);

private:
	static void Report(int line, const std::string& where, const std::string& message);

	static bool m_HadError;
};
