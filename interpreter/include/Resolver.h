#pragma once
#include "Stmt.h"
#include "Expr.h"
#include "Interpreter.h"
#include <vector>
#include <unordered_map>
#include <string>

class Resolver : public ExprVisitor, public StmtVisitor {
    private:
        Interpreter* interpreter;
        std::vector<std::unordered_map<std::string, bool>> scopes;
        void resolve(Stmt* stmt);
        void resolve(Expr* expr);
        void beginScope();
        void endScope();
        void declare(Token name);
        void define(Token name);
        void resolveLocal(Expr* expr, Token name);

    public:
        Resolver(Interpreter* interpreter);
        void resolve(const std::vector<std::unique_ptr<Stmt>>& statements);
        void visitBlockStmt(BlockStmt* stmt);
        void visitVarDeclStmt(VarDeclStmt* stmt);
        std::any visitVariableExpr(VariableExpr* expr);
        std::any visitAssignExpr(AssignExpr* expr);
        void visitFunctionStmt(FunctionStmt* stmt);
        std::any visitLiteralExpr(LiteralExpr* expr);
        std::any visitBinaryExpr(BinaryExpr* expr);
        std::any visitUnaryExpr(UnaryExpr* expr);
        std::any visitPrefixPostfixExpr(PrefixPostfixExpr* expr);
        std::any visitLogicalExpr(LogicalExpr* expr);
        std::any visitTernaryExpr(TernaryExpr* expr);
        void visitExpressionStmt(ExpressionStmt* stmt);
        std::any visitCallExpr(CallExpr* expr);
        std::any visitMethodCallExpr(MethodCallExpr* expr);
        std::any visitIndexAccessExpr(IndexAccessExpr* expr);
        std::any visitIndexAssignExpr(IndexAssignExpr* expr);
        std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr);
        std::any visitInstantiateExpr(InstantiateExpr* expr);
        void visitIfStmt(IfStmt* stmt);
        void visitWhileStmt(WhileStmt* stmt);
        void visitForStmt(ForStmt* stmt);
        void visitBreakStmt(BreakStmt* stmt);
        void visitContinueStmt(ContinueStmt* stmt);
        void visitReturnStmt(ReturnStmt* stmt);
        void visitSwitchStmt(SwitchStmt* stmt);
};