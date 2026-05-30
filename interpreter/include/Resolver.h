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

        void resolve(Stmt* stmt) {
            stmt->accept(this);
        }

        void resolve(Expr* expr) {
            expr->accept(this);
        }

        void beginScope() {
            scopes.emplace_back(std::unordered_map<std::string, bool>());
        }

        void endScope() {
            scopes.pop_back();
        }

        void declare(Token name) {
            if(scopes.empty()) return;
            auto& scope = scopes.back();

            if(scope.find(name.lexeme) != scope.end()) {
                throw RuntimeError(name, "Já existe uma variável com o nome: '" + name.lexeme + "' neste escopo.");
            }

            scope[name.lexeme] = false;
        }

        void define(Token name) {
            if(scopes.empty()) return;
            scopes.back()[name.lexeme] = true;
        }

        void resolveLocal(Expr* expr, Token name) {
            for(int i = scopes.size() - 1; i >= 0; i--) {
                if(scopes[i].find(name.lexeme) != scopes[i].end()) {
                    interpreter->resolve(expr, scopes.size() - 1 - i);
                    return;
                }
            }
        }

    public:
        Resolver(Interpreter* interpreter) : interpreter(interpreter) {}

        void resolve(const std::vector<std::unique_ptr<Stmt>>& statements) {
            for(const auto& statement : statements) {
                resolve(statement.get());
            }
        }

        void visitBlockStmt(BlockStmt* stmt) override {
            beginScope();
            for(const auto& statement : stmt->statements) {
                resolve(statement.get());
            }
            endScope();
        }

        void visitVarDeclStmt(VarDeclStmt* stmt) override {
            declare(stmt->name);
            if(stmt->initializer != nullptr) {
                resolve(stmt->initializer.get());
            }
            define(stmt->name);
        }

        std::any visitVariableExpr(VariableExpr* expr) override {
            if(!scopes.empty()) {
                auto& scope = scopes.back();
                if(scope.find(expr->name.lexeme) != scope.end() && scope[expr->name.lexeme] == false) {
                    throw RuntimeError(expr->name, "Não é possível ler a variável durante sua inicialização.");
                }
            }
            resolveLocal(expr, expr->name);
            return {};
        }

        std::any visitAssignExpr(AssignExpr* expr) override {
            resolve(expr->value.get());
            resolveLocal(expr, expr->name);
            return {};
        }

        void visitFunctionStmt(FunctionStmt* stmt) override {
            declare(stmt->name);
            define(stmt->name);
            beginScope();
            for(const auto& param : stmt->params) {
                declare(param.name);
                define(param.name);
            }
            for(const auto& statement : stmt->body) {
                resolve(statement.get());
            }
            endScope();
        }

        std::any visitLiteralExpr(LiteralExpr* expr) override {
            return {};
        }

        std::any visitBinaryExpr(BinaryExpr* expr) override {
            resolve(expr->left.get());
            resolve(expr->right.get());
            return {};
        }

        std::any visitUnaryExpr(UnaryExpr* expr) override {
            resolve(expr->right.get());
            return {};
        }

        std::any visitPrefixPostfixExpr(PrefixPostfixExpr* expr) override {
            resolve(expr->target.get());
            return {};
        }

        std::any visitLogicalExpr(LogicalExpr* expr) override {
            resolve(expr->left.get());
            resolve(expr->right.get());
            return {};
        }

        std::any visitTernaryExpr(TernaryExpr* expr) override {
            resolve(expr->condition.get());
            resolve(expr->trueExpr.get());
            resolve(expr->falseExpr.get());
            return {};
        }

        void visitExpressionStmt(ExpressionStmt* stmt) override {
            resolve(stmt->expression.get());
        }

        std::any visitCallExpr(CallExpr* expr) override {
            resolve(expr->callee.get());
            for(const auto& arg : expr->arguments) {
                resolve(arg.get());
            }
            return {};
        }

        std::any visitMethodCallExpr(MethodCallExpr* expr) override {
            resolve(expr->object.get());
            for(const auto& arg : expr->arguments) {
                resolve(arg.get());
            }
            return {};
        }

        std::any visitIndexAccessExpr(IndexAccessExpr* expr) override {
            resolve(expr->target.get());
            resolve(expr->index.get());
            return {};
        }

        std::any visitIndexAssignExpr(IndexAssignExpr* expr) override {
            resolve(expr->target.get());
            resolve(expr->index.get());
            resolve(expr->value.get());
            return {};
        }

        std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr) override {
            for(const auto& element : expr->elements) {
                resolve(element.get());
            }
            return {};
        }

        std::any visitInstantiateExpr(InstantiateExpr* expr) override {
            resolve(expr->size.get());
            resolve(expr->initialValue.get());
            return {};
        }

        void visitIfStmt(IfStmt* stmt) override {
            resolve(stmt->condition.get());
            resolve(stmt->thenBranch.get());
            if(stmt->elseBranch != nullptr) resolve(stmt->elseBranch.get());
        }

        void visitWhileStmt(WhileStmt* stmt) override {
            resolve(stmt->condition.get());
            resolve(stmt->body.get());
        }

        void visitForStmt(ForStmt* stmt) override {
            beginScope();
            if(stmt->initializer != nullptr) resolve(stmt->initializer.get());
            if(stmt->condition != nullptr) resolve(stmt->condition.get());
            if(stmt->increment != nullptr) resolve(stmt->increment.get());
            resolve(stmt->body.get());
            endScope();
        }

        void visitBreakStmt(BreakStmt* stmt) override {
        }

        void visitContinueStmt(ContinueStmt* stmt) override {
        }

        void visitReturnStmt(ReturnStmt* stmt) override {
            if(stmt->value != nullptr) {
                resolve(stmt->value.get());
            }
        }

        void visitSwitchStmt(SwitchStmt* stmt) override {
            resolve(stmt->target.get());
            for(const auto& switchCase : stmt->cases) {
                if(switchCase.matchExpr != nullptr) {
                    resolve(switchCase.matchExpr.get());
                }

                beginScope();
                for(const auto& statement : switchCase.body) {
                    resolve(statement.get());
                }
                endScope();
            }
        }
};