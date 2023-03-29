#include "Clock.h"
#include <chrono>

Clock::Clock()
{
}

std::any Clock::Call(const Interpreter&, const std::vector<std::any>&)
{
	return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()).count());
}

size_t Clock::Arity()
{
	return 0;
}
