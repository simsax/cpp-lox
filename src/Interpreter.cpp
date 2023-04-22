#include "Interpreter.h"
#include "Clock.h"
#include "Lox.h"
#include "LoxCallable.h"
#include "LoxClass.h"
#include "LoxInstance.h"
#include <memory>
#include <regex>

Interpreter::Interpreter()
    : m_Globals(std::make_shared<Environment>())
    , m_CurrentEnvironment(m_Globals)
{
    // std::any can hold only copy constructible types -> no unique_ptr...
    m_Globals->Define("clock", static_cast<std::shared_ptr<LoxCallable>>(std::make_shared<Clock>()));
}

void Interpreter::Interpret(const std::vector<std::unique_ptr<stmt::Stmt>>& statements)
{
    try {
        for (const auto& statement : statements) {
            Execute(statement.get());
        }
    } catch (const RuntimeException& ex) {
        Lox::RuntimeError(ex.GetToken(), ex.what());
    }
}

void Interpreter::Resolve(expr::Expr* expr, int depth)
{
    m_Locals.insert({ expr, depth });
}

std::any Interpreter::VisitAssign(expr::Assign* expr)
{
    std::any assignmentValue = Evaluate(expr->m_Value.get());

    if (m_Locals.contains(expr)) {
        int distance = m_Locals.at(expr);
        m_CurrentEnvironment->AssignAt(distance, expr->m_Name, assignmentValue);
    } else {
        m_Globals->Assign(expr->m_Name, assignmentValue);
    }

    return assignmentValue;
}

std::any Interpreter::VisitBinary(expr::Binary* expr)
{
    std::any left = Evaluate(expr->m_Left.get());
    std::any right = Evaluate(expr->m_Right.get());

    switch (expr->m_Opr.type) {
    case TokenType::MINUS:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) - std::any_cast<double>(right);
    case TokenType::PLUS:
        if (left.type() == typeid(double) && right.type() == typeid(double))
            return std::any_cast<double>(left) + std::any_cast<double>(right);
        else if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
            return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
        else
            throw RuntimeException("Operands must be two numbers or two strings.", expr->m_Opr);
    case TokenType::SLASH:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) / std::any_cast<double>(right);
    case TokenType::STAR:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) * std::any_cast<double>(right);
    case TokenType::GREATER:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) > std::any_cast<double>(right);
    case TokenType::GREATER_EQUAL:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) >= std::any_cast<double>(right);
    case TokenType::LESS:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) < std::any_cast<double>(right);
    case TokenType::LESS_EQUAL:
        CheckNumberOperands(expr->m_Opr, left, right);
        return std::any_cast<double>(left) <= std::any_cast<double>(right);
    case TokenType::EQUAL_EQUAL:
        return IsEqual(left, right);
    case TokenType::BANG_EQUAL:
        return !IsEqual(left, right);
    default:
        break;
    }

    // unreachable
    return nullptr;
}

std::any Interpreter::VisitLogical(expr::Logical* expr)
{
    std::any left = Evaluate(expr->m_Left.get());
    if (expr->m_Opr.type == TokenType::OR) {
        if (IsTruthy(left))
            return left;
    } else {
        if (!IsTruthy(left))
            return left;
    }
    return Evaluate(expr->m_Right.get());
}

std::any Interpreter::VisitGrouping(expr::Grouping* expr)
{
    return Evaluate(expr->m_Expression.get());
}

std::any Interpreter::VisitLiteral(expr::Literal* expr)
{
    return expr->m_Value;
}

std::any Interpreter::VisitUnary(expr::Unary* expr)
{
    // post order traversal: each node evaluates the children before doing its own work
    std::any right = Evaluate(expr->m_Right.get());
    switch (expr->m_Opr.type) {
    case TokenType::BANG:
        return !(IsTruthy(right));
    case TokenType::MINUS:
        CheckNumberOperand(expr->m_Opr, right);
        return -std::any_cast<double>(right);
    default:
        break;
    }
    // unreachable
    return nullptr;
}

std::any Interpreter::VisitVariable(expr::Variable* expr)
{
    return LookUpVariable(expr->m_Name, expr);
}

std::any Interpreter::VisitCall(expr::Call* expr)
{
    std::any callee = Evaluate(expr->m_Callee.get());
    std::vector<std::any> arguments;
    arguments.reserve(expr->m_Arguments.size());

    for (const auto& argument : expr->m_Arguments) {
        arguments.emplace_back(Evaluate(argument.get()));
    }

    if (callee.type() == typeid(std::shared_ptr<LoxCallable>)) {
        std::shared_ptr<LoxCallable> function = std::any_cast<std::shared_ptr<LoxCallable>>(callee);
        if (arguments.size() != function->Arity()) {
            throw RuntimeException("Expected " + std::to_string(function->Arity())
                    + " arguments but got " + std::to_string(arguments.size()) + ".",
                expr->m_Paren);
        }
        return function->Call(*this, arguments);
    } else {
        throw RuntimeException("Can only call function and classes.", expr->m_Paren);
    }
}

std::any Interpreter::VisitGet(expr::Get* expr)
{
    std::any object = Evaluate(expr->m_Object.get());
    // only instances have properties
    if (object.type() == typeid(std::shared_ptr<LoxInstance>)) {
        std::shared_ptr<LoxInstance> instance = std::any_cast<std::shared_ptr<LoxInstance>>(object);
        return instance->Get(expr->m_Name);
    }

    throw RuntimeException("Only instances have properties.", expr->m_Name);
}

std::any Interpreter::VisitSet(expr::Set* expr)
{
    /*
                    1. Evaluate the object.
                    2. Raise a runtime error if it's not an instance of a class.
                    3. Evaluate the value.
    */
    std::any object = Evaluate(expr->m_Object.get());
    if (object.type() == typeid(std::shared_ptr<LoxInstance>)) {
        std::shared_ptr<LoxInstance> instance = std::any_cast<std::shared_ptr<LoxInstance>>(object);
        std::any value = Evaluate(expr->m_Value.get());
        instance->Set(expr->m_Name, value);
        return value;
    } else {
        throw RuntimeException("Only instances have fields.", expr->m_Name);
    }
}

std::any Interpreter::VisitExpression(stmt::Expression* stmt)
{
    Evaluate(stmt->m_Expression.get());
    return nullptr;
}

std::any Interpreter::VisitPrint(stmt::Print* stmt)
{
    std::any value = Evaluate(stmt->m_Expression.get());
    std::cout << ToString(value) << "\n";
    return nullptr;
}

std::any Interpreter::VisitVar(stmt::Var* stmt)
{
    std::any value = nullptr;
    expr::Expr* initializer = stmt->m_Initializer.get();
    if (initializer != nullptr)
        value = Evaluate(initializer);
    m_CurrentEnvironment->Define(stmt->m_Name.lexeme, std::move(value));
    return nullptr;
}

std::any Interpreter::VisitBlock(stmt::Block* stmt)
{
    ExecuteBlock(stmt->m_Statements, std::make_shared<Environment>(m_CurrentEnvironment));
    return nullptr;
}

std::any Interpreter::VisitIf(stmt::If* stmt)
{
    if (IsTruthy(Evaluate(stmt->m_Condition.get()))) {
        Execute(stmt->m_ThenBranch.get());
    } else if (stmt->m_ElseBranch.get() != nullptr) {
        Execute(stmt->m_ElseBranch.get());
    }
    return nullptr;
}

std::any Interpreter::VisitWhile(stmt::While* stmt)
{
    while (IsTruthy(Evaluate(stmt->m_Condition.get())))
        Execute(stmt->m_Statement.get());
    return nullptr;
}

std::any Interpreter::VisitFunction(stmt::Function* stmt)
{
    std::shared_ptr<LoxCallable> loxFunction = std::make_shared<LoxFunction>(
        stmt, m_CurrentEnvironment, false);
    m_CurrentEnvironment->Define(stmt->m_Name.lexeme, loxFunction);
    return nullptr;
}

std::any Interpreter::VisitReturn(stmt::Return* stmt)
{
    std::any value = nullptr;
    if (stmt->m_Expression.get() != nullptr)
        value = Evaluate(stmt->m_Expression.get());
    throw Return(value);
}

std::any Interpreter::VisitClass(stmt::Class* stmt)
{
    m_CurrentEnvironment->Define(stmt->m_Name.lexeme, nullptr);
    MethodMap methods;
    for (const std::unique_ptr<stmt::Function>& method : stmt->m_Methods) {
        methods.insert({ method->m_Name.lexeme,
            std::make_shared<LoxFunction>(method.get(), m_CurrentEnvironment,
                method->m_Name.lexeme == "init") });
    }
    std::shared_ptr<LoxCallable> klass = std::make_shared<LoxClass>(
        stmt->m_Name.lexeme, std::move(methods));
    m_CurrentEnvironment->Assign(stmt->m_Name, klass);
    return nullptr;
}

std::any Interpreter::VisitThis(expr::This* expr)
{
    return LookUpVariable(expr->m_Keyword, expr);
}

void Interpreter::CheckNumberOperand(const Token& opr, const std::any& operand) const
{
    if (operand.type() == typeid(double))
        return;
    throw RuntimeException("Operand must be a number.", opr);
}

void Interpreter::CheckNumberOperands(const Token& opr, const std::any& left, const std::any& right) const
{
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return;
    throw RuntimeException("Operands must be numbers.", opr);
}

std::any Interpreter::Evaluate(expr::Expr* expr)
{
    return expr->Accept(*this);
}

void Interpreter::Execute(stmt::Stmt* statement)
{
    statement->Accept(*this);
}

void Interpreter::ExecuteBlock(const std::vector<std::unique_ptr<stmt::Stmt>>& statements,
    std::shared_ptr<Environment> environment)
{
    std::shared_ptr<Environment> previous = m_CurrentEnvironment;
    m_CurrentEnvironment = environment;

    try {
        for (const auto& statement : statements) {
            Execute(statement.get());
        }
        m_CurrentEnvironment = previous;
    } catch (const Return& returnValue) {
        m_CurrentEnvironment = previous;
        throw returnValue;
    } catch (const RuntimeException& ex) {
        m_CurrentEnvironment = previous;
        throw ex;
    }
}

bool Interpreter::IsTruthy(std::any value) const
{
    if (value.type() == typeid(bool))
        return std::any_cast<bool>(value);
    if (value.type() == typeid(std::nullptr_t))
        return false;
    return true;
}

bool Interpreter::IsEqual(std::any left, std::any right) const
{
    if (left.type() != right.type())
        return false;
    if (left.type() == typeid(bool) && right.type() == typeid(bool))
        return std::any_cast<bool>(left) == std::any_cast<bool>(right);
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return std::any_cast<double>(left) == std::any_cast<double>(right);
    if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
        return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
    if (left.type() == typeid(std::nullptr_t) && right.type() == typeid(std::nullptr_t))
        return true;
    return false;
}

std::any Interpreter::LookUpVariable(const Token& name, expr::Expr* expr) const
{
    if (m_Locals.contains(expr)) {
        int distance = m_Locals.at(expr);
        return m_CurrentEnvironment->GetAt(distance, name.lexeme);
    } else {
        return m_Globals->Get(name);
    }
}

std::string Interpreter::ToString(const std::any& value) const
{
    if (value.type() == typeid(int)) {
        int val = std::any_cast<int>(value);
        return std::to_string(val);
    } else if (value.type() == typeid(double)) {
        double val = std::any_cast<double>(value);
        std::string doubleString = std::to_string(val);
        return std::regex_replace(doubleString, std::regex { "\\.0+$" }, "");
    } else if (value.type() == typeid(bool)) {
        bool val = std::any_cast<bool>(value);
        if (val)
            return "true";
        else
            return "false";
    } else if (value.type() == typeid(nullptr)) {
        return "nil";
    } else if (value.type() == typeid(std::shared_ptr<LoxCallable>)) {
        std::shared_ptr<LoxCallable> val = std::any_cast<std::shared_ptr<LoxCallable>>(value);
        return val->ToString();
    } else if (value.type() == typeid(std::shared_ptr<LoxInstance>)) {
        std::shared_ptr<LoxInstance> val = std::any_cast<std::shared_ptr<LoxInstance>>(value);
        return val->ToString();
    } else {
        return std::any_cast<std::string>(value);
    }
}
