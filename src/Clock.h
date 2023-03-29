#pragma once
#include "LoxCallable.h"

class Clock : public LoxCallable {
public:
	Clock();
	std::any Call(const Interpreter&, const std::vector<std::any>&) override;
	size_t Arity() override;
};