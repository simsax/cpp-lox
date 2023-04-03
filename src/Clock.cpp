#include "Clock.h"
#include <chrono>
#include <cmath>

Clock::Clock()
{
}

std::any Clock::Call(Interpreter&, const std::vector<std::any>&)
{
	return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count()) * std::pow(10, -9);
}

size_t Clock::Arity()
{
	return 0;
}

std::string Clock::ToString()
{
	return "<built-in fn clock>";
}
