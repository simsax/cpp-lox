#pragma once
#include "Environment.h"
#include "Expr.h"
#include "Stmt.h"
#include <stdexcept>
#include <vector>

class RuntimeException : public std::runtime_error {
public:
    RuntimeException(const std::string& error, const Token& token)
        : std::runtime_error(error.c_str())
        , m_Token(token)
    {
    }

    inline const Token& GetToken() const { return m_Token; }

private:
    Token m_Token;
};

class Return : public std::runtime_error {
public:
    Return(const std::any& value)
        : std::runtime_error("")
        , m_Value(value)
    {
    }

    inline std::any GetValue() const { return m_Value; }

private:
    std::any m_Value;
};

class Interpreter : public expr::Visitor, public stmt::Visitor {
public:
    Interpreter();

    void Interpret(const std::vector<std::unique_ptr<stmt::Stmt>>& statements);
    void Resolve(expr::Expr* expr, int depth);

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
    std::any VisitThis(expr::This* expr) override;
    std::any VisitSuper(expr::Super* expr) override;

    std::any VisitExpression(stmt::Expression* stmt) override;
    std::any VisitPrint(stmt::Print* stmt) override;
    std::any VisitVar(stmt::Var* stmt) override;
    std::any VisitBlock(stmt::Block* stmt) override;
    std::any VisitIf(stmt::If* stmt) override;
    std::any VisitWhile(stmt::While* stmt) override;
    std::any VisitFunction(stmt::Function* stmt) override;
    std::any VisitReturn(stmt::Return* stmt) override;
    std::any VisitClass(stmt::Class* stmt) override;

    friend class LoxFunction;

private:
    void CheckNumberOperand(const Token& opr, const std::any& operand) const;
    void CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const;
    std::string ToString(const std::any& value) const;

    std::any Evaluate(expr::Expr* expr);
    void Execute(stmt::Stmt* statement);
    void ExecuteBlock(const std::vector<std::unique_ptr<stmt::Stmt>>& statements,
        std::shared_ptr<Environment> environment);
    bool IsTruthy(std::any value) const;
    bool IsEqual(std::any left, std::any right) const;
    std::any LookUpVariable(const Token& name, expr::Expr* expr) const;

    std::shared_ptr<Environment> m_Globals;
    std::shared_ptr<Environment> m_CurrentEnvironment;
    std::unordered_map<expr::Expr*, int> m_Locals;
};