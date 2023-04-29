#include "Interpreter.h"
#include "Clock.h"
#include "Lox.h"
#include "LoxCallable.h"
#include "LoxClass.h"
#include "LoxInstance.h"
#include <memory>
#include <regex>
#include <stdint.h>

Interpreter::Interpreter()
    : m_Globals(std::make_shared<Environment>())
    , m_CurrentEnvironment(m_Globals)
{
    // std::any can hold only copy constructible types -> no unique_ptr...
    m_Globals->Define(
        "clock", static_cast<std::shared_ptr<LoxCallable>>(std::make_shared<Clock>()));
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

void Interpreter::Resolve(expr::Expr* expr, uint32_t depth, uint32_t variableIndex)
{
    m_Locals.insert({ expr, { depth, variableIndex } });
}

void Interpreter::Interpret(expr::Expr* expression)
{
    try {
        std::any value = Evaluate(expression);
        std::cout << ToString(value) << "\n";
    } catch (const RuntimeException& ex) {
        Lox::RuntimeError(ex.GetToken(), ex.what());
    }
}

std::any Interpreter::AssignVariable(
    const Token& name, expr::Expr* expr, const std::any& assignmentValue) const
{
    if (m_Locals.contains(expr)) {
        ResolvedData resolvedData = m_Locals.at(expr);
        uint32_t distance = resolvedData.scopeHops;
        uint32_t variableIndex = resolvedData.variableIndex;
        m_CurrentEnvironment->AssignAt(distance, variableIndex, assignmentValue);
    } else {
        m_Globals->Assign(name, assignmentValue);
    }

    return assignmentValue;
}

std::any Interpreter::VisitAssign(expr::Assign* expr)
{
    std::any assignmentValue = Evaluate(expr->m_Value.get());
    return AssignVariable(expr->m_Name, expr, assignmentValue);
}

std::any Interpreter::VisitOprAssign(expr::OprAssign* expr)
{
    std::any variableValue = LookUpVariable(expr->m_Name, expr);
    std::any assignmentValue = Evaluate(expr->m_Value.get());
    switch (expr->m_Opr.type) {
    case TokenType::MINUS_EQUAL:
        variableValue = Subtract(expr->m_Opr, variableValue, assignmentValue);
        break;
    case TokenType::PLUS_EQUAL:
        variableValue = Add(expr->m_Opr, variableValue, assignmentValue);
        break;
    case TokenType::SLASH_EQUAL:
        variableValue = Divide(expr->m_Opr, variableValue, assignmentValue);
        break;
    case TokenType::STAR_EQUAL:
        variableValue = Multiply(expr->m_Opr, variableValue, assignmentValue);
        break;
    default:
        break;
    }
    return AssignVariable(expr->m_Name, expr, variableValue);
}

std::any Interpreter::VisitTernary(expr::Ternary* expr)
{
    if (IsTruthy(Evaluate(expr->m_Condition.get())))
        return Evaluate(expr->m_ThenBranch.get());
    else
        return Evaluate(expr->m_ElseBranch.get());
}

std::any Interpreter::VisitAnonFunction(expr::AnonFunction* expr)
{
    return static_cast<std::shared_ptr<LoxCallable>>(
        std::make_shared<LoxAnonFunction>(expr, m_CurrentEnvironment));
}

std::any Interpreter::VisitBinary(expr::Binary* expr)
{
    std::any left = Evaluate(expr->m_Left.get());
    std::any right = Evaluate(expr->m_Right.get());

    switch (expr->m_Opr.type) {
    case TokenType::COMMA:
        return right;
    case TokenType::MINUS:
        return Subtract(expr->m_Opr, left, right);
    case TokenType::PLUS:
        return Add(expr->m_Opr, left, right);
    case TokenType::SLASH:
        return Divide(expr->m_Opr, left, right);
    case TokenType::STAR:
        return Multiply(expr->m_Opr, left, right);
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

std::any Interpreter::VisitLiteral(expr::Literal* expr) { return expr->m_Value; }

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
    if (object.type() == typeid(std::shared_ptr<LoxCallable>)) {
        std::shared_ptr<LoxCallable> classCallable
            = std::any_cast<std::shared_ptr<LoxCallable>>(object);
        // if it's a class calling its static method
        if (auto klassObj = dynamic_cast<LoxInstance*>(classCallable.get())) {
            return klassObj->Get(expr->m_Name);
        }
    }
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

std::any Interpreter::VisitOprSet(expr::OprSet* expr)
{
    std::any object = Evaluate(expr->m_Object.get());
    if (object.type() == typeid(std::shared_ptr<LoxInstance>)) {
        std::shared_ptr<LoxInstance> instance = std::any_cast<std::shared_ptr<LoxInstance>>(object);
        std::any assignmentValue = Evaluate(expr->m_Value.get());
        // in this case the field has to exist (otherwise raise an error),
        // and I get its value from the instance
        std::any fieldValue = instance->Get(expr->m_Name);

        switch (expr->m_Opr.type) {
        case TokenType::MINUS_EQUAL:
            fieldValue = Subtract(expr->m_Opr, fieldValue, assignmentValue);
            break;
        case TokenType::PLUS_EQUAL:
            fieldValue = Add(expr->m_Opr, fieldValue, assignmentValue);
            break;
        case TokenType::SLASH_EQUAL:
            fieldValue = Divide(expr->m_Opr, fieldValue, assignmentValue);
            break;
        case TokenType::STAR_EQUAL:
            fieldValue = Multiply(expr->m_Opr, fieldValue, assignmentValue);
            break;
        default:
            // unreachable code
            throw RuntimeException(
                "Operator '" + expr->m_Opr.lexeme + "' is not valid.", expr->m_Opr);
            break;
        }

        instance->Set(expr->m_Name, fieldValue);
        return fieldValue;
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
    if (m_CurrentEnvironment == m_Globals)
        m_CurrentEnvironment->Define(stmt->m_Name.lexeme, std::move(value));
    else
        m_CurrentEnvironment->DefineLocal(std::move(value));
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
    while (IsTruthy(Evaluate(stmt->m_Condition.get()))) {
        try {
            Execute(stmt->m_Statement.get());
        } catch (const JumpException& ex) {
            if (ex.GetToken().type == TokenType::BREAK)
                break;
            else
                continue;
        }
    }
    return nullptr;
}

std::any Interpreter::VisitFor(stmt::For* stmt)
{
    std::shared_ptr<Environment> localEnvironment
        = std::make_shared<Environment>(m_CurrentEnvironment);
    std::shared_ptr<Environment> previous = m_CurrentEnvironment;
    m_CurrentEnvironment = localEnvironment;

    try {
        if (stmt->m_Initializer.get() != nullptr) {
            Execute(stmt->m_Initializer.get());
        }
        expr::Expr* increment = stmt->m_Increment.get();
        while (IsTruthy(Evaluate(stmt->m_Condition.get()))) {
            try {
                Execute(stmt->m_Body.get());
                if (increment != nullptr)
                    Evaluate(increment);
            } catch (const JumpException& ex) {
                if (ex.GetToken().type == TokenType::BREAK)
                    break;
                else {
                    if (increment != nullptr)
                        Evaluate(increment);
                    continue;
                }
            }
        }
        m_CurrentEnvironment = previous;
    } catch (const RuntimeException& ex) {
        m_CurrentEnvironment = previous;
        throw ex;
    }
    return nullptr;
}

std::any Interpreter::VisitFunction(stmt::Function* stmt)
{
    std::shared_ptr<LoxCallable> loxFunction
        = std::make_shared<LoxFunction>(stmt, m_CurrentEnvironment, false);
    if (m_CurrentEnvironment == m_Globals)
        m_CurrentEnvironment->Define(stmt->m_Name.lexeme, loxFunction);
    else
        m_CurrentEnvironment->DefineLocal(loxFunction);
    return nullptr;
}

std::any Interpreter::VisitJump(stmt::Jump* stmt) { throw JumpException(stmt->m_Name); }

std::any Interpreter::Add(const Token& opr, const std::any& left, const std::any& right)
{
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return std::any_cast<double>(left) + std::any_cast<double>(right);
    else if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
        return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
    else if (left.type() == typeid(std::string))
        return std::any_cast<std::string>(left) + ToString(right);
    else if (right.type() == typeid(std::string))
        return ToString(left) + std::any_cast<std::string>(right);
    else
        throw RuntimeException("Operands must be two numbers or two strings.", opr);
}

std::any Interpreter::Subtract(const Token& opr, const std::any& left, const std::any& right)
{
    CheckNumberOperands(opr, left, right);
    return std::any_cast<double>(left) - std::any_cast<double>(right);
}

std::any Interpreter::Multiply(const Token& opr, const std::any& left, const std::any& right)
{
    CheckNumberOperands(opr, left, right);
    return std::any_cast<double>(left) * std::any_cast<double>(right);
}

std::any Interpreter::Divide(const Token& opr, const std::any& left, const std::any& right)
{
    CheckNumberOperands(opr, left, right);
    if (std::any_cast<double>(right) == 0)
        throw DivisionByZeroException("Right operand cannot be zero.", opr);
    return std::any_cast<double>(left) / std::any_cast<double>(right);
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
    MethodMap classMethods;
    for (const std::unique_ptr<stmt::Function>& method : stmt->m_Methods) {
        methods.insert({ method->m_Name.lexeme,
            std::make_shared<LoxFunction>(
                method.get(), m_CurrentEnvironment, method->m_Name.lexeme == "init") });
    }
    for (const std::unique_ptr<stmt::Function>& method : stmt->m_ClassMethods) {
        classMethods.insert({ method->m_Name.lexeme,
            std::make_shared<LoxFunction>(method.get(), m_CurrentEnvironment, false) });
    }
    std::shared_ptr<LoxClass> metaClass = std::make_shared<LoxClass>(
        stmt->m_Name.lexeme + " metaclass", std::move(classMethods), std::shared_ptr<LoxClass> {});

    std::shared_ptr<LoxCallable> klass
        = std::make_shared<LoxClass>(stmt->m_Name.lexeme, std::move(methods), std::move(metaClass));
    m_CurrentEnvironment->Assign(stmt->m_Name, klass);
    return nullptr;
}

std::any Interpreter::VisitThis(expr::This* expr) { return LookUpVariable(expr->m_Keyword, expr); }

void Interpreter::CheckNumberOperand(const Token& opr, const std::any& operand) const
{
    if (operand.type() == typeid(double))
        return;
    throw RuntimeException("Operand must be a number.", opr);
}

void Interpreter::CheckNumberOperands(
    const Token& opr, const std::any& left, const std::any& right) const
{
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return;
    throw RuntimeException("Operands must be numbers.", opr);
}

std::any Interpreter::Evaluate(expr::Expr* expr) { return expr->Accept(*this); }

void Interpreter::Execute(stmt::Stmt* statement) { statement->Accept(*this); }

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
    } catch (const JumpException& ex) {
        m_CurrentEnvironment = previous;
        throw ex;
    } catch (const RuntimeException& ex) {
        m_CurrentEnvironment = previous;
        throw ex;
    }
}

bool Interpreter::IsTruthy(const std::any& value) const
{
    if (value.type() == typeid(bool))
        return std::any_cast<bool>(value);
    if (value.type() == typeid(std::nullptr_t))
        return false;
    return true;
}

bool Interpreter::IsEqual(const std::any& left, const std::any& right) const
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
        ResolvedData resolvedData = m_Locals.at(expr);
        uint32_t distance = resolvedData.scopeHops;
        uint32_t variableIndex = resolvedData.variableIndex;
        return m_CurrentEnvironment->GetAt(distance, variableIndex);
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
