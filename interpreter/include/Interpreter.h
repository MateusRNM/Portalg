#pragma once
#include "Expr.h"
#include "Stmt.h"
#include "Environment.h"
#include "Callables.h"
#include "PortalgTypes.h"
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <cmath>
#include <unordered_map>

class Interpreter : public ExprVisitor, public StmtVisitor {
private:
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    int loopDepth = 0;
    int switchDepth = 0;
    int functionDepth = 0;

    PortalgValue evaluate(Expr* expr);
    bool isReal(const PortalgValue& operand);
    bool isInteger(const PortalgValue& operand);
    double getAsDouble(const PortalgValue& operand);
    bool isEqual(const PortalgValue& a, const PortalgValue& b);
    PortalgValue calculateMathAndRelationals(const PortalgValue& left, Token op, const PortalgValue& right);
    bool matchType(PortalgValue& value, const std::vector<Token>& typeTokens, size_t& index);
    std::vector<Token> extractSubtype(const std::vector<Token>& fullType);
    void validateType(const std::vector<Token>& typeTokens, PortalgValue& value, Token errorToken);
    void enforceType(const std::vector<Token>& typeTokens, PortalgValue& value, Token errorToken);
    PortalgValue applyIncrementDecrement(const PortalgValue& currentValue, TokenType opType, Token errorToken);
    PortalgValue cloneValue(const PortalgValue& value);
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
        PortalgValue call(Interpreter* interpreter, const std::vector<PortalgValue>& arguments) override;
    };

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);
    void execute(Stmt* stmt);
    std::string stringify(const PortalgValue& operand);
    PortalgValue visitLiteralExpr(LiteralExpr* expr) override;
    PortalgValue visitBinaryExpr(BinaryExpr* expr) override;
    PortalgValue visitUnaryExpr(UnaryExpr* expr) override;
    PortalgValue visitPrefixPostfixExpr(PrefixPostfixExpr* expr) override;
    PortalgValue visitVariableExpr(VariableExpr* expr) override;
    PortalgValue visitLogicalExpr(LogicalExpr* expr) override;
    PortalgValue visitTernaryExpr(TernaryExpr* expr) override;
    PortalgValue visitAssignExpr(AssignExpr* expr) override;
    PortalgValue visitCallExpr(CallExpr* expr) override;
    PortalgValue visitMethodCallExpr(MethodCallExpr* expr) override;
    PortalgValue visitIndexAccessExpr(IndexAccessExpr* expr) override;
    PortalgValue visitIndexAssignExpr(IndexAssignExpr* expr) override;
    PortalgValue visitArrayLiteralExpr(ArrayLiteralExpr* expr) override;
    PortalgValue visitInstantiateExpr(InstantiateExpr* expr) override;
    void visitExpressionStmt(ExpressionStmt* stmt) override;
    void visitVarDeclStmt(VarDeclStmt* stmt) override;
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