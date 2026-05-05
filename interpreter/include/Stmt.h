#pragma once
#include "Expr.h"

class ExpressionStmt;
class VarDeclStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class BreakStmt;
class ContinueStmt;
class ReturnStmt;
class FunctionStmt;
class SwitchStmt;

class StmtVisitor {
public:
    virtual void visitExpressionStmt(ExpressionStmt* stmt) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt* stmt) = 0;
    virtual void visitBlockStmt(BlockStmt* stmt) = 0;
    virtual void visitIfStmt(IfStmt* stmt) = 0;
    virtual void visitWhileStmt(WhileStmt* stmt) = 0;
    virtual void visitForStmt(ForStmt* stmt) = 0;
    virtual void visitBreakStmt(BreakStmt* stmt) = 0;
    virtual void visitContinueStmt(ContinueStmt* stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
    virtual void visitFunctionStmt(FunctionStmt* stmt) = 0;
    virtual void visitSwitchStmt(SwitchStmt* stmt) = 0;
    virtual ~StmtVisitor() = default;
};

class Stmt {
public:
    virtual void accept(StmtVisitor* visitor) = 0;
    virtual ~Stmt() = default;
};

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    ExpressionStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitExpressionStmt(this); }
};

class VarDeclStmt : public Stmt {
public:
    std::vector<Token> type;
    Token name;
    bool isConst;
    std::unique_ptr<Expr> initializer;
    VarDeclStmt(std::vector<Token> type, Token name, bool isConst, std::unique_ptr<Expr> initializer) 
        : type(type), name(name), isConst(isConst), initializer(std::move(initializer)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitVarDeclStmt(this); }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) : statements(std::move(statements)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitBlockStmt(this); }
};

class IfStmt : public Stmt {
public:
    Token ifToken;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch, Token ifToken) 
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)), ifToken(ifToken) {}
    void accept(StmtVisitor* visitor) override { visitor->visitIfStmt(this); }
};

class WhileStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body, Token keyword) 
        : condition(std::move(condition)), body(std::move(body)), keyword(keyword) {}
    void accept(StmtVisitor* visitor) override { visitor->visitWhileStmt(this); }
};

class ForStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;
    ForStmt(std::unique_ptr<Stmt> init, std::unique_ptr<Expr> cond, std::unique_ptr<Expr> inc, std::unique_ptr<Stmt> body, Token keyword)
        : initializer(std::move(init)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(body)), keyword(keyword) {}
    void accept(StmtVisitor* visitor) override { visitor->visitForStmt(this); }
};

class BreakStmt : public Stmt {
public:
    Token keyword;
    BreakStmt(Token keyword) : keyword(keyword) {}
    void accept(StmtVisitor* visitor) override { visitor->visitBreakStmt(this); }
};

class ContinueStmt : public Stmt {
public:
    Token keyword;
    ContinueStmt(Token keyword) : keyword(keyword) {}
    void accept(StmtVisitor* visitor) override { visitor->visitContinueStmt(this); }
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;
    ReturnStmt(Token keyword, std::unique_ptr<Expr> value) : keyword(keyword), value(std::move(value)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitReturnStmt(this); }
};

struct FunctionParam {
    std::vector<Token> type;
    Token name;
    bool isReference;
};

class FunctionStmt : public Stmt {
public:
    std::vector<Token> returnType;
    Token name;
    std::vector<FunctionParam> params;
    std::vector<std::unique_ptr<Stmt>> body;
    FunctionStmt(std::vector<Token> returnType, Token name, std::vector<FunctionParam> params, std::vector<std::unique_ptr<Stmt>> body) 
        : returnType(returnType), name(name), params(std::move(params)), body(std::move(body)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitFunctionStmt(this); }
};

struct CaseClause {
    std::unique_ptr<Expr> matchExpr;
    std::vector<std::unique_ptr<Stmt>> body;
};

class SwitchStmt : public Stmt {
public:
    std::unique_ptr<Expr> target;
    std::vector<CaseClause> cases;
    SwitchStmt(std::unique_ptr<Expr> target, std::vector<CaseClause> cases) 
        : target(std::move(target)), cases(std::move(cases)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitSwitchStmt(this); }
};