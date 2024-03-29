#include "Resolver.h"
#include "Lox.h"
#include <cstddef>

Resolver::Resolver(Interpreter* interpreter)
    : m_Interpreter(interpreter)
    , m_CurrentFunction(FunctionType::NONE)
    , m_CurrentClass(ClassType::NONE)
{
}

std::any Resolver::VisitAssign(expr::Assign* expr)
{
    Resolve(expr->m_Value.get());
    ResolveLocal(expr, expr->m_Name);
    return nullptr;
}

std::any Resolver::VisitBinary(expr::Binary* expr)
{
    Resolve(expr->m_Left.get());
    Resolve(expr->m_Right.get());
    return nullptr;
}

std::any Resolver::VisitLogical(expr::Logical* expr)
{
    Resolve(expr->m_Left.get());
    Resolve(expr->m_Right.get());
    return nullptr;
}

std::any Resolver::VisitGrouping(expr::Grouping* expr)
{
    Resolve(expr->m_Expression.get());
    return nullptr;
}

std::any Resolver::VisitLiteral(expr::Literal*) { return nullptr; }

std::any Resolver::VisitUnary(expr::Unary* expr)
{
    Resolve(expr->m_Right.get());
    return nullptr;
}

std::any Resolver::VisitVariable(expr::Variable* expr)
{
    if (!m_Scopes.empty() && m_Scopes.back().contains(expr->m_Name.lexeme)
        && m_Scopes.back().at(expr->m_Name.lexeme) == false) {
        Lox::Error(expr->m_Name, "Can't read local variable in its own initializer.");
    }
    ResolveLocal(expr, expr->m_Name);
    return nullptr;
}

std::any Resolver::VisitCall(expr::Call* expr)
{
    Resolve(expr->m_Callee.get());
    for (const auto& argument : expr->m_Arguments) {
        Resolve(argument.get());
    }
    return nullptr;
}

std::any Resolver::VisitExpression(stmt::Expression* stmt)
{
    Resolve(stmt->m_Expression.get());
    return nullptr;
}

std::any Resolver::VisitPrint(stmt::Print* stmt)
{
    Resolve(stmt->m_Expression.get());
    return nullptr;
}

std::any Resolver::VisitVar(stmt::Var* stmt)
{
    Declare(stmt->m_Name);
    expr::Expr* initializer = stmt->m_Initializer.get();
    if (initializer != nullptr)
        Resolve(initializer);
    Define(stmt->m_Name);
    return nullptr;
}

std::any Resolver::VisitBlock(stmt::Block* stmt)
{
    BeginScope();
    Resolve(stmt->m_Statements);
    EndScope();
    return nullptr;
}

std::any Resolver::VisitIf(stmt::If* stmt)
{
    Resolve(stmt->m_Condition.get());
    Resolve(stmt->m_ThenBranch.get());
    if (stmt->m_ElseBranch.get() != nullptr)
        Resolve(stmt->m_ElseBranch.get());
    return nullptr;
}

std::any Resolver::VisitWhile(stmt::While* stmt)
{
    Resolve(stmt->m_Condition.get());
    Resolve(stmt->m_Statement.get());
    return nullptr;
}

std::any Resolver::VisitFunction(stmt::Function* stmt)
{
    Declare(stmt->m_Name);
    Define(stmt->m_Name);
    ResolveFunction(stmt, FunctionType::FUNCTION);
    return nullptr;
}

std::any Resolver::VisitReturn(stmt::Return* stmt)
{
    if (m_CurrentFunction == FunctionType::NONE) {
        Lox::Error(stmt->m_Keyword, "Can't return from top-level code.");
    }
    if (stmt->m_Expression.get() != nullptr) {
        if (m_CurrentFunction == FunctionType::INITIALIZER) {
            Lox::Error(stmt->m_Keyword, "Can't return a value from initializer.");
        }
        Resolve(stmt->m_Expression.get());
    }
    return nullptr;
}

std::any Resolver::VisitClass(stmt::Class* stmt)
{
    ClassType enclosingClass = m_CurrentClass;
    m_CurrentClass = ClassType::CLASS;
    Declare(stmt->m_Name);
    Define(stmt->m_Name);
    if (stmt->m_SuperClass != nullptr) {
        m_CurrentClass = ClassType::SUBCLASS;
        if (stmt->m_Name.lexeme == stmt->m_SuperClass->m_Name.lexeme) {
            Lox::Error(stmt->m_SuperClass->m_Name, "A class can't inherit from itself.");
        }
        Resolve(stmt->m_SuperClass.get());
        BeginScope();
        m_Scopes.back().insert({ "super", true });
    }

    BeginScope();
    m_Scopes.back().insert({ "this", true });
    for (const auto& method : stmt->m_Methods) {
        FunctionType declaration = FunctionType::METHOD;
        if (method->m_Name.lexeme == "init") {
            declaration = FunctionType::INITIALIZER;
        }
        ResolveFunction(method.get(), declaration);
    }
    EndScope();
    if (stmt->m_SuperClass != nullptr) {
        EndScope();
    }
    m_CurrentClass = enclosingClass;
    return nullptr;
}

std::any Resolver::VisitGet(expr::Get* expr)
{
    Resolve(expr->m_Object.get());
    return nullptr;
}

std::any Resolver::VisitSet(expr::Set* expr)
{
    Resolve(expr->m_Value.get());
    Resolve(expr->m_Object.get());
    return nullptr;
}

std::any Resolver::VisitThis(expr::This* expr)
{
    if (m_CurrentClass == ClassType::NONE) {
        Lox::Error(expr->m_Keyword, "Can't use 'this' outside of a class.");
    }
    ResolveLocal(expr, expr->m_Keyword);
    return nullptr;
}

std::any Resolver::VisitSuper(expr::Super* expr)
{
    if (m_CurrentClass == ClassType::NONE) {
        Lox::Error(expr->m_Keyword, "Can't use 'super' outside of a class.");
    } else if (m_CurrentClass != ClassType::SUBCLASS) {
        Lox::Error(expr->m_Keyword, "Can't use 'super' in a class with no superclass.");
    }
    ResolveLocal(expr, expr->m_Keyword);
    return nullptr;
}

void Resolver::BeginScope() { m_Scopes.push_back(std::unordered_map<std::string, bool> {}); }

void Resolver::EndScope() { m_Scopes.pop_back(); }

void Resolver::Resolve(const std::vector<std::unique_ptr<stmt::Stmt>>& statements)
{
    for (auto& statement : statements) {
        Resolve(statement.get());
    }
}

void Resolver::Resolve(stmt::Stmt* stmt) { stmt->Accept(*this); }

void Resolver::ResolveFunction(stmt::Function* function, FunctionType funcType)
{
    FunctionType enclosingFunction = m_CurrentFunction;
    m_CurrentFunction = funcType;
    BeginScope();
    for (const Token& param : function->m_Params) {
        Declare(param);
        Define(param);
    }
    Resolve(function->m_Body);
    EndScope();
    m_CurrentFunction = enclosingFunction;
}

void Resolver::Resolve(expr::Expr* expr) { expr->Accept(*this); }

void Resolver::Declare(const Token& name)
{
    if (m_Scopes.empty())
        return;
    std::unordered_map<std::string, bool>& scope = m_Scopes.back();
    if (scope.contains(name.lexeme)) {
        Lox::Error(name, "Already a variable with this name in this scope.");
    }
    scope.insert({ name.lexeme, false });
}

void Resolver::Define(const Token& name)
{
    if (m_Scopes.empty())
        return;
    std::unordered_map<std::string, bool>& scope = m_Scopes.back();
    scope[name.lexeme] = true;
}

void Resolver::ResolveLocal(expr::Expr* expr, const Token& name)
{
    int numScopes = static_cast<int>(m_Scopes.size());
    for (int i = numScopes - 1; i >= 0; i--) {
        if (m_Scopes[i].contains(name.lexeme)) {
            // if the variable is in the current scope, we pass 0.
            // if it's in the immediately enclosing scope, we pass 1, and so on
            m_Interpreter->Resolve(expr, numScopes - 1 - i);
            return;
        }
    }
}
