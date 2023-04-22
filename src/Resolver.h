#pragma once

#include "Interpreter.h"

class Resolver : public expr::Visitor, public stmt::Visitor {
public:
    Resolver(Interpreter* interpreter);

    void Resolve(const std::vector<std::unique_ptr<stmt::Stmt>>& statements);

    std::any VisitAssign(expr::Assign* expr) override;
    std::any VisitBinary(expr::Binary* expr) override;
    std::any VisitLogical(expr::Logical* expr) override;
    std::any VisitGrouping(expr::Grouping* expr) override;
    std::any VisitLiteral(expr::Literal* expr) override;
    std::any VisitUnary(expr::Unary* expr) override;
    std::any VisitVariable(expr::Variable* expr) override;
    std::any VisitCall(expr::Call* expr) override;
    std::any VisitGet(expr::Get* expr) override;
    std::any VisitSet(expr::Set* expr) override;

    std::any VisitExpression(stmt::Expression* stmt) override;
    std::any VisitPrint(stmt::Print* stmt) override;
    std::any VisitVar(stmt::Var* stmt) override;
    std::any VisitBlock(stmt::Block* stmt) override;
    std::any VisitIf(stmt::If* stmt) override;
    std::any VisitWhile(stmt::While* stmt) override;
    std::any VisitFunction(stmt::Function* stmt) override;
    std::any VisitReturn(stmt::Return* stmt) override;
    std::any VisitClass(stmt::Class* stmt) override;

private:
    enum class FunctionType : uint8_t {
        FUNCTION,
        METHOD,
        NONE
    };

    void Resolve(stmt::Stmt* stmt);
    void BeginScope();
    void EndScope();
    void ResolveFunction(stmt::Function* function, FunctionType funcType);
    void Resolve(expr::Expr* expr);
    void Declare(const Token& name);
    void Define(const Token& name);
    void ResolveLocal(expr::Expr* expr, const Token& name);

    Interpreter* m_Interpreter;
    std::vector<std::unordered_map<std::string, bool>> m_Scopes;
    FunctionType m_CurrentFunction;
};