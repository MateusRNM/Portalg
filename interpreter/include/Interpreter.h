#pragma once
#include "Expr.h"
#include "Stmt.h"
#include "Environment.h"
#include "Callables.h"
#include <any>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <cmath>

struct TypedArray {
    std::shared_ptr<std::vector<Token>> typeTokens;
    std::shared_ptr<std::vector<std::any>> elements;
};

class Interpreter : public ExprVisitor, public StmtVisitor {
private:
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    int loopDepth = 0;
    int switchDepth = 0;
    int functionDepth = 0;

    std::any evaluate(Expr* expr);
    bool isReal(const std::any& operand);
    bool isInteger(const std::any& operand);
    double getAsDouble(const std::any& operand);
    bool isEqual(const std::any& a, const std::any& b);
    std::any calculateMathAndRelationals(const std::any& left, Token op, const std::any& right);
    bool matchType(std::any& value, const std::vector<Token>& typeTokens, size_t& index);
    std::vector<Token> extractSubtype(const std::vector<Token>& fullType);
    void validateType(const std::vector<Token>& typeTokens, std::any& value, Token errorToken);
    void enforceType(const std::vector<Token>& typeTokens, std::any& value, Token errorToken);
    std::any applyIncrementDecrement(const std::any& currentValue, TokenType opType, Token errorToken);
    std::any cloneValue(const std::any& value);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> innerEnvironment);

public:
    Interpreter();

    std::unordered_map<Expr*, int> locals;
    bool debugModeOn = false;

    void resolve(Expr* expr, int depth);

    struct PortalgUserFunction : public PortalgCallable {
        FunctionStmt* declaration;
        std::shared_ptr<Environment> closure;

        PortalgUserFunction(FunctionStmt* declaration, std::shared_ptr<Environment> closure);

        int arity() override;
        std::any call(Interpreter* interpreter, const std::vector<std::any>& arguments) override;
    };

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);
    void execute(Stmt* stmt);
    std::string stringify(const std::any& operand);

    std::any visitLiteralExpr(LiteralExpr* expr) override;
    std::any visitBinaryExpr(BinaryExpr* expr) override;
    std::any visitUnaryExpr(UnaryExpr* expr) override;
    std::any visitPrefixPostfixExpr(PrefixPostfixExpr* expr) override;
    std::any visitVariableExpr(VariableExpr* expr) override;
    std::any visitLogicalExpr(LogicalExpr* expr) override;
    std::any visitTernaryExpr(TernaryExpr* expr) override;
    void visitExpressionStmt(ExpressionStmt* stmt) override;
    void visitVarDeclStmt(VarDeclStmt* stmt) override;
    std::any visitAssignExpr(AssignExpr* expr) override;
    std::any visitCallExpr(CallExpr* expr) override;
    std::any visitMethodCallExpr(MethodCallExpr* expr) override;
    std::any visitIndexAccessExpr(IndexAccessExpr* expr) override;
    std::any visitIndexAssignExpr(IndexAssignExpr* expr) override;
    std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr) override;
    std::any visitInstantiateExpr(InstantiateExpr* expr) override;
    void visitBlockStmt(BlockStmt* stmt) override;
    void visitIfStmt(IfStmt* stmt) override;
    void visitWhileStmt(WhileStmt* stmt) override;
    void visitForStmt(ForStmt* stmt) override;
    void visitBreakStmt(BreakStmt* stmt) override;
    void visitContinueStmt(ContinueStmt* stmt) override;
    void visitReturnStmt(ReturnStmt* stmt) override;
    void visitFunctionStmt(FunctionStmt* stmt) override;
    void visitSwitchStmt(SwitchStmt* stmt) override;
};