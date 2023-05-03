#pragma once
#include "Token.h"
#include <any>
#include <memory>
#include <vector>

namespace stmt {
struct Stmt;
}

namespace expr {

class Visitor;

struct Expr {
    virtual ~Expr() = 0;

    virtual std::any Accept(Visitor& visitor) = 0;
};

inline Expr::~Expr() = default;

struct Assign : public Expr {
    Assign(const Token& name, std::unique_ptr<Expr> value)
        : m_Name(name)
        , m_Value(std::move(value))
    {
    }

    std::any Accept(Visitor& visitor) override;

    Token m_Name;
    std::unique_ptr<Expr> m_Value;
};

struct OprAssign : public Expr {
    OprAssign(const Token& name, const Token& opr, std::unique_ptr<Expr> value)
        : m_Name(name)
        , m_Opr(opr)
        , m_Value(std::move(value))
    {
    }

    std::any Accept(Visitor& visitor) override;

    Token m_Name;
    Token m_Opr;
    std::unique_ptr<Expr> m_Value;
};

struct Logical : public Expr {
    Logical(std::unique_ptr<Expr> left, const Token& opr, std::unique_ptr<Expr> right)
        : m_Left(std::move(left))
        , m_Opr(opr)
        , m_Right(std::move(right))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Left;
    Token m_Opr;
    std::unique_ptr<Expr> m_Right;
};

struct Binary : public Expr {
    Binary(std::unique_ptr<Expr> left, const Token& opr, std::unique_ptr<Expr> right)
        : m_Left(std::move(left))
        , m_Opr(opr)
        , m_Right(std::move(right))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Left;
    Token m_Opr;
    std::unique_ptr<Expr> m_Right;
};

struct Grouping : public Expr {
    Grouping(std::unique_ptr<Expr> expression)
        : m_Expression(std::move(expression))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Expression;
};

struct Literal : public Expr {
    Literal(const std::any& value)
        : m_Value(value)
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::any m_Value;
};

struct Unary : public Expr {
    Unary(const Token& opr, std::unique_ptr<Expr> right)
        : m_Opr(opr)
        , m_Right(std::move(right))
    {
    }

    std::any Accept(Visitor& visitor) override;

    Token m_Opr;
    std::unique_ptr<Expr> m_Right;
};

struct Variable : public Expr {
    Variable(const Token& name)
        : m_Name(name)
    {
    }

    std::any Accept(Visitor& visitor) override;

    Token m_Name;
};

struct Ternary : public Expr {
    Ternary(std::unique_ptr<expr::Expr> condition, std::unique_ptr<expr::Expr> thenBranch,
        std::unique_ptr<expr::Expr> elseBranch)
        : m_Condition(std::move(condition))
        , m_ThenBranch(std::move(thenBranch))
        , m_ElseBranch(std::move(elseBranch))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<expr::Expr> m_Condition;
    std::unique_ptr<expr::Expr> m_ThenBranch;
    std::unique_ptr<expr::Expr> m_ElseBranch;
};

struct AnonFunction : public Expr {
    AnonFunction(const std::vector<Token>& params, std::vector<std::unique_ptr<stmt::Stmt>> body);

    virtual ~AnonFunction() override;

    std::any Accept(Visitor& visitor) override;

    std::vector<Token> m_Params;
    std::vector<std::unique_ptr<stmt::Stmt>> m_Body;
};

struct Call : public Expr {
    Call(std::unique_ptr<Expr> callee, const Token& paren,
        std::vector<std::unique_ptr<Expr>> arguments)
        : m_Callee(std::move(callee))
        , m_Paren(paren)
        , m_Arguments(std::move(arguments))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Callee;
    Token m_Paren;
    std::vector<std::unique_ptr<Expr>> m_Arguments;
};

struct Get : public Expr {
    Get(std::unique_ptr<Expr> object, const Token& name)
        : m_Object(std::move(object))
        , m_Name(name)
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Object;
    Token m_Name;
};

struct Set : public Expr {
    Set(std::unique_ptr<Expr> object, const Token& name, std::unique_ptr<Expr> value)
        : m_Object(std::move(object))
        , m_Name(name)
        , m_Value(std::move(value))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Object;
    Token m_Name;
    std::unique_ptr<Expr> m_Value;
};

struct OprSet : public Expr {
    OprSet(std::unique_ptr<Expr> object, const Token& name, const Token& opr,
        std::unique_ptr<Expr> value)
        : m_Object(std::move(object))
        , m_Name(name)
        , m_Opr(opr)
        , m_Value(std::move(value))
    {
    }

    std::any Accept(Visitor& visitor) override;

    std::unique_ptr<Expr> m_Object;
    Token m_Name;
    Token m_Opr;
    std::unique_ptr<Expr> m_Value;
};

struct This : public Expr {
    This(const Token& keyword)
        : m_Keyword(keyword)
    {
    }

    std::any Accept(Visitor& visitor) override;

    Token m_Keyword;
};

struct Super : public Expr {
    Super(const Token& keyword, const Token& method)
        : m_Keyword(keyword)
        , m_Method(method)
    {
    }

    std::any Accept(Visitor& visitor) override;

    Token m_Keyword;
    Token m_Method;
};

class Visitor {
public:
    virtual ~Visitor() = 0;
    virtual std::any VisitAssign(Assign* expr) = 0;
    virtual std::any VisitBinary(Binary* expr) = 0;
    virtual std::any VisitLogical(Logical* expr) = 0;
    virtual std::any VisitGrouping(Grouping* expr) = 0;
    virtual std::any VisitLiteral(Literal* expr) = 0;
    virtual std::any VisitUnary(Unary* expr) = 0;
    virtual std::any VisitVariable(Variable* expr) = 0;
    virtual std::any VisitCall(Call* expr) = 0;
    virtual std::any VisitOprAssign(OprAssign* expr) = 0;
    virtual std::any VisitTernary(Ternary* expr) = 0;
    virtual std::any VisitAnonFunction(AnonFunction* expr) = 0;
    virtual std::any VisitGet(Get* expr) = 0;
    virtual std::any VisitSet(Set* expr) = 0;
    virtual std::any VisitThis(This* expr) = 0;
    virtual std::any VisitOprSet(OprSet* expr) = 0;
    virtual std::any VisitSuper(Super* expr) = 0;
};

inline Visitor::~Visitor() = default;

inline std::any Assign::Accept(Visitor& visitor) { return visitor.VisitAssign(this); }

inline std::any Binary::Accept(Visitor& visitor) { return visitor.VisitBinary(this); }

inline std::any Logical::Accept(Visitor& visitor) { return visitor.VisitLogical(this); }

inline std::any Grouping::Accept(Visitor& visitor) { return visitor.VisitGrouping(this); }

inline std::any Literal::Accept(Visitor& visitor) { return visitor.VisitLiteral(this); }

inline std::any Unary::Accept(Visitor& visitor) { return visitor.VisitUnary(this); }

inline std::any Variable::Accept(Visitor& visitor) { return visitor.VisitVariable(this); }

inline std::any Call::Accept(Visitor& visitor) { return visitor.VisitCall(this); }

inline std::any OprAssign::Accept(Visitor& visitor) { return visitor.VisitOprAssign(this); }

inline std::any Ternary::Accept(Visitor& visitor) { return visitor.VisitTernary(this); }

inline std::any AnonFunction::Accept(Visitor& visitor) { return visitor.VisitAnonFunction(this); }

inline std::any Get::Accept(Visitor& visitor) { return visitor.VisitGet(this); }

inline std::any Set::Accept(Visitor& visitor) { return visitor.VisitSet(this); }

inline std::any This::Accept(Visitor& visitor) { return visitor.VisitThis(this); }

inline std::any OprSet::Accept(Visitor& visitor) { return visitor.VisitOprSet(this); }
inline std::any Super::Accept(Visitor& visitor) { return visitor.VisitSuper(this); }
}
