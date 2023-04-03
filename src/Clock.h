#pragma once
#include "LoxCallable.h"

class Clock : public LoxCallable {
public:
	Clock();
	std::any Call(Interpreter&, const std::vector<std::any>&) override;
	size_t Arity() override;
	std::string ToString() override;
};