#pragma once
#include "Environment.h"
#include "LoxAnonFunction.h"
#include "LoxFunction.h"
#include "Stmt.h"
#include <stdexcept>
#include <stdint.h>
#include <vector>

class RuntimeException : public std::runtime_error {
public:
    RuntimeException(const std::string& error, const Token& token)
        : std::runtime_error(error.c_str())
        , m_Token(token)
    {
    }

    inline const Token& GetToken() const { return m_Token; }

protected:
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

class DivisionByZeroException : public RuntimeException {
public:
    DivisionByZeroException(const std::string& error, const Token& token)
        : RuntimeException(error, token)
    {
    }
};

class JumpException : public RuntimeException {
public:
    JumpException(const Token& token)
        : RuntimeException("", token)
    {
    }
};

class Interpreter : public expr::Visitor, public stmt::Visitor {
public:
    Interpreter();

    void Interpret(const std::vector<std::unique_ptr<stmt::Stmt>>& statements);
    void Resolve(expr::Expr* expr, uint32_t depth, uint32_t variableIndex);
    void Interpret(expr::Expr* expression);

    std::any VisitAssign(expr::Assign* expr) override;
    std::any VisitBinary(expr::Binary* expr) override;
    std::any VisitLogical(expr::Logical* expr) override;
    std::any VisitGrouping(expr::Grouping* expr) override;
    std::any VisitLiteral(expr::Literal* expr) override;
    std::any VisitUnary(expr::Unary* expr) override;
    std::any VisitVariable(expr::Variable* expr) override;
    std::any VisitCall(expr::Call* expr) override;
    std::any VisitOprAssign(expr::OprAssign* expr) override;
    std::any VisitTernary(expr::Ternary* expr) override;
    std::any VisitAnonFunction(expr::AnonFunction* expr) override;
    std::any VisitGet(expr::Get* expr) override;
    std::any VisitSet(expr::Set* expr) override;
    std::any VisitThis(expr::This* expr) override;
    std::any VisitOprSet(expr::OprSet* expr) override;

    std::any VisitExpression(stmt::Expression* stmt) override;
    std::any VisitPrint(stmt::Print* stmt) override;
    std::any VisitVar(stmt::Var* stmt) override;
    std::any VisitBlock(stmt::Block* stmt) override;
    std::any VisitIf(stmt::If* stmt) override;
    std::any VisitWhile(stmt::While* stmt) override;
    std::any VisitFunction(stmt::Function* stmt) override;
    std::any VisitReturn(stmt::Return* stmt) override;
    std::any VisitFor(stmt::For* stmt) override;
    std::any VisitJump(stmt::Jump* stmt) override;
    std::any VisitClass(stmt::Class* stmt) override;

    std::any Add(const Token& opr, const std::any& left, const std::any& right);
    std::any Subtract(const Token& opr, const std::any& left, const std::any& right);
    std::any Multiply(const Token& opr, const std::any& left, const std::any& right);
    std::any Divide(const Token& opr, const std::any& left, const std::any& right);

    friend class LoxFunction;
    friend class LoxAnonFunction;

private:
    struct ResolvedData {
        uint32_t scopeHops;
        uint32_t variableIndex;
    };

    void CheckNumberOperand(const Token& opr, const std::any& operand) const;
    void CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const;
    std::string ToString(const std::any& value) const;

    std::any Evaluate(expr::Expr* expr);
    void Execute(stmt::Stmt* statement);
    void ExecuteBlock(const std::vector<std::unique_ptr<stmt::Stmt>>& statements,
        std::shared_ptr<Environment> environment);
    bool IsTruthy(const std::any& value) const;
    bool IsEqual(const std::any& left, const std::any& right) const;
    std::any LookUpVariable(const Token& name, expr::Expr* expr) const;
    std::any AssignVariable(
        const Token& name, expr::Expr* expr, const std::any& assignmentValue) const;

    std::shared_ptr<Environment> m_Globals;
    std::shared_ptr<Environment> m_CurrentEnvironment;

    // map that associates each syntax tree node with its resolved data
    // a map using the variable names as keys wouldn't work because the same variable
    // can be accessed in different expressions and in different scopes
    std::unordered_map<expr::Expr*, ResolvedData> m_Locals;
};