#include "Expr.h"
#include "Stmt.h"

// needed for this reason: https://ortogonal.github.io/cpp/forward-declaration-and-smart-pointers/

namespace expr {

	AnonFunction::AnonFunction(const std::vector<Token>& params,
		std::vector<std::unique_ptr<stmt::Stmt>> body) :
		m_Params(params),
		m_Body(std::move(body))
	{ }

	AnonFunction::~AnonFunction() = default;

}