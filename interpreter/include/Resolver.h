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
        PortalgValue visitVariableExpr(VariableExpr* expr);
        PortalgValue visitAssignExpr(AssignExpr* expr);
        void visitFunctionStmt(FunctionStmt* stmt);
        PortalgValue visitLiteralExpr(LiteralExpr* expr);
        PortalgValue visitBinaryExpr(BinaryExpr* expr);
        PortalgValue visitUnaryExpr(UnaryExpr* expr);
        PortalgValue visitPrefixPostfixExpr(PrefixPostfixExpr* expr);
        PortalgValue visitLogicalExpr(LogicalExpr* expr);
        PortalgValue visitTernaryExpr(TernaryExpr* expr);
        void visitExpressionStmt(ExpressionStmt* stmt);
        PortalgValue visitCallExpr(CallExpr* expr);
        PortalgValue visitMethodCallExpr(MethodCallExpr* expr);
        PortalgValue visitIndexAccessExpr(IndexAccessExpr* expr);
        PortalgValue visitIndexAssignExpr(IndexAssignExpr* expr);
        PortalgValue visitArrayLiteralExpr(ArrayLiteralExpr* expr);
        PortalgValue visitInstantiateExpr(InstantiateExpr* expr);
        void visitIfStmt(IfStmt* stmt);
        void visitWhileStmt(WhileStmt* stmt);
        void visitForStmt(ForStmt* stmt);
        void visitBreakStmt(BreakStmt* stmt);
        void visitContinueStmt(ContinueStmt* stmt);
        void visitReturnStmt(ReturnStmt* stmt);
        void visitSwitchStmt(SwitchStmt* stmt);
};