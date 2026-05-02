#pragma once
#include <any>
#include <memory>
#include <vector>
#include "Token.h"

class BinaryExpr;
class LogicalExpr;
class UnaryExpr;
class LiteralExpr;
class VariableExpr;
class AssignExpr;
class TernaryExpr;
class CallExpr;
class MethodCallExpr;
class IndexAccessExpr;
class IndexAssignExpr;
class PrefixPostfixExpr;
class ArrayLiteralExpr;

class ExprVisitor {
public:
    virtual std::any visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual std::any visitLogicalExpr(LogicalExpr* expr) = 0;
    virtual std::any visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual std::any visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual std::any visitVariableExpr(VariableExpr* expr) = 0;
    virtual std::any visitAssignExpr(AssignExpr* expr) = 0;
    virtual std::any visitTernaryExpr(TernaryExpr* expr) = 0;
    virtual std::any visitCallExpr(CallExpr* expr) = 0;
    virtual std::any visitMethodCallExpr(MethodCallExpr* expr) = 0;
    virtual std::any visitIndexAccessExpr(IndexAccessExpr* expr) = 0;
    virtual std::any visitIndexAssignExpr(IndexAssignExpr* expr) = 0;
    virtual std::any visitPrefixPostfixExpr(PrefixPostfixExpr* expr) = 0;
    virtual std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr) = 0;
    virtual std::any visitInstantiateExpr(InstantiateExpr* expr) = 0;
    virtual ~ExprVisitor() = default;
};

class Expr {
public:
    virtual std::any accept(ExprVisitor* visitor) = 0;
    virtual ~Expr() = default;
};

class LiteralExpr : public Expr {
public:
    std::any value;
    LiteralExpr(std::any value) : value(value) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitLiteralExpr(this); }
};

class VariableExpr : public Expr {
public:
    Token name;
    VariableExpr(Token name) : name(name) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitVariableExpr(this); }
};

class AssignExpr : public Expr {
public:
    Token name;
    Token op; 
    std::unique_ptr<Expr> value;
    AssignExpr(Token name, Token op, std::unique_ptr<Expr> value) : name(name), op(op), value(std::move(value)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitAssignExpr(this); }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right) 
        : left(std::move(left)), op(op), right(std::move(right)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitBinaryExpr(this); }
};

class LogicalExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    LogicalExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right) 
        : left(std::move(left)), op(op), right(std::move(right)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitLogicalExpr(this); }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitUnaryExpr(this); }
};

class PrefixPostfixExpr : public Expr {
public:
    std::unique_ptr<Expr> target;
    Token op;
    bool isPrefix; 
    PrefixPostfixExpr(std::unique_ptr<Expr> target, Token op, bool isPrefix) : target(std::move(target)), op(op), isPrefix(isPrefix) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitPrefixPostfixExpr(this); }
};

class TernaryExpr : public Expr {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> trueExpr;
    std::unique_ptr<Expr> falseExpr;
    TernaryExpr(std::unique_ptr<Expr> cond, std::unique_ptr<Expr> t, std::unique_ptr<Expr> f) 
        : condition(std::move(cond)), trueExpr(std::move(t)), falseExpr(std::move(f)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitTernaryExpr(this); }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitCallExpr(this); }
};

class MethodCallExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token methodName;
    std::vector<std::unique_ptr<Expr>> arguments;
    MethodCallExpr(std::unique_ptr<Expr> object, Token methodName, std::vector<std::unique_ptr<Expr>> arguments) 
        : object(std::move(object)), methodName(methodName), arguments(std::move(arguments)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitMethodCallExpr(this); }
};

class IndexAccessExpr : public Expr {
public:
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> index;
    IndexAccessExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> index) : target(std::move(target)), index(std::move(index)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitIndexAccessExpr(this); }
};

class IndexAssignExpr : public Expr {
public:
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> index;
    Token op;
    std::unique_ptr<Expr> value;
    IndexAssignExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> index, Token op, std::unique_ptr<Expr> value) 
        : target(std::move(target)), index(std::move(index)), op(op), value(std::move(value)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitIndexAssignExpr(this); }
};

class ArrayLiteralExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elements) : elements(std::move(elements)) {}
    std::any accept(ExprVisitor* visitor) override { return visitor->visitArrayLiteralExpr(this); }
};

class InstantiateExpr : public Expr {
public:
    std::vector<Token> type;
    std::unique_ptr<Expr> size;
    std::unique_ptr<Expr> initialValue;

    InstantiateExpr(std::vector<Token> type, std::unique_ptr<Expr> size, std::unique_ptr<Expr> initialValue)
        : type(type), size(std::move(size)), initialValue(std::move(initialValue)) {}

    std::any accept(ExprVisitor* visitor) override {
        return visitor->visitInstantiateExpr(this);
    }
};