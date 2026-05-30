#include "Resolver.h"

void Resolver::resolve(Stmt* stmt) {
    stmt->accept(this);
}

void Resolver::resolve(Expr* expr) {
    expr->accept(this);
}

void Resolver::beginScope() {
    scopes.emplace_back(std::unordered_map<std::string, bool>());
}

void Resolver::endScope() {
    scopes.pop_back();
}

void Resolver::declare(Token name) {
    if(scopes.empty()) return;
    auto& scope = scopes.back();

    if(scope.find(name.lexeme) != scope.end()) {
        throw RuntimeError(name, "Já existe uma variável com o nome: '" + name.lexeme + "' neste escopo.");
    }

    scope[name.lexeme] = false;
}

void Resolver::define(Token name) {
    if(scopes.empty()) return;
    scopes.back()[name.lexeme] = true;
}

void Resolver::resolveLocal(Expr* expr, Token name) {
    for(int i = scopes.size() - 1; i >= 0; i--) {
        if(scopes[i].find(name.lexeme) != scopes[i].end()) {
            interpreter->resolve(expr, scopes.size() - 1 - i);
            return;
        }
    }
}

Resolver::Resolver(Interpreter* interpreter) : interpreter(interpreter) {}

void Resolver::resolve(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for(const auto& statement : statements) {
        resolve(statement.get());
    }
}

void Resolver::visitBlockStmt(BlockStmt* stmt) {
    beginScope();
    for(const auto& statement : stmt->statements) {
        resolve(statement.get());
    }
    endScope();
}

void Resolver::visitVarDeclStmt(VarDeclStmt* stmt) {
    declare(stmt->name);
    if(stmt->initializer != nullptr) {
        resolve(stmt->initializer.get());
    }
    define(stmt->name);
}

std::any Resolver::visitVariableExpr(VariableExpr* expr) {
    if(!scopes.empty()) {
        auto& scope = scopes.back();
        if(scope.find(expr->name.lexeme) != scope.end() && scope[expr->name.lexeme] == false) {
            throw RuntimeError(expr->name, "Não é possível ler a variável durante sua inicialização.");
        }
    }
    resolveLocal(expr, expr->name);
    return {};
}

std::any Resolver::visitAssignExpr(AssignExpr* expr) {
    resolve(expr->value.get());
    resolveLocal(expr, expr->name);
    return {};
}

void Resolver::visitFunctionStmt(FunctionStmt* stmt) {
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

std::any Resolver::visitLiteralExpr(LiteralExpr* expr) {
    return {};
}

std::any Resolver::visitBinaryExpr(BinaryExpr* expr) {
    resolve(expr->left.get());
    resolve(expr->right.get());
    return {};
}

std::any Resolver::visitUnaryExpr(UnaryExpr* expr) {
    resolve(expr->right.get());
    return {};
}

std::any Resolver::visitPrefixPostfixExpr(PrefixPostfixExpr* expr) {
    resolve(expr->target.get());
    return {};
}

std::any Resolver::visitLogicalExpr(LogicalExpr* expr) {
    resolve(expr->left.get());
    resolve(expr->right.get());
    return {};
}

std::any Resolver::visitTernaryExpr(TernaryExpr* expr) {
    resolve(expr->condition.get());
    resolve(expr->trueExpr.get());
    resolve(expr->falseExpr.get());
    return {};
}

void Resolver::visitExpressionStmt(ExpressionStmt* stmt) {
    resolve(stmt->expression.get());
}

std::any Resolver::visitCallExpr(CallExpr* expr) {
    resolve(expr->callee.get());
    for(const auto& arg : expr->arguments) {
        resolve(arg.get());
    }
    return {};
}

std::any Resolver::visitMethodCallExpr(MethodCallExpr* expr) {
    resolve(expr->object.get());
    for(const auto& arg : expr->arguments) {
        resolve(arg.get());
    }
    return {};
}

std::any Resolver::visitIndexAccessExpr(IndexAccessExpr* expr) {
    resolve(expr->target.get());
    resolve(expr->index.get());
    return {};
}

std::any Resolver::visitIndexAssignExpr(IndexAssignExpr* expr) {
    resolve(expr->target.get());
    resolve(expr->index.get());
    resolve(expr->value.get());
    return {};
}

std::any Resolver::visitArrayLiteralExpr(ArrayLiteralExpr* expr) {
    for(const auto& element : expr->elements) {
        resolve(element.get());
    }
    return {};
}

std::any Resolver::visitInstantiateExpr(InstantiateExpr* expr) {
    resolve(expr->size.get());
    resolve(expr->initialValue.get());
    return {};
}

void Resolver::visitIfStmt(IfStmt* stmt) {
    resolve(stmt->condition.get());
    resolve(stmt->thenBranch.get());
    if(stmt->elseBranch != nullptr) resolve(stmt->elseBranch.get());
}

void Resolver::visitWhileStmt(WhileStmt* stmt) {
    resolve(stmt->condition.get());
    resolve(stmt->body.get());
}

void Resolver::visitForStmt(ForStmt* stmt) {
    beginScope();
    if(stmt->initializer != nullptr) resolve(stmt->initializer.get());
    if(stmt->condition != nullptr) resolve(stmt->condition.get());
    if(stmt->increment != nullptr) resolve(stmt->increment.get());
    resolve(stmt->body.get());
    endScope();
}

void Resolver::visitBreakStmt(BreakStmt* stmt) {
}

void Resolver::visitContinueStmt(ContinueStmt* stmt) {
}

void Resolver::visitReturnStmt(ReturnStmt* stmt) {
    if(stmt->value != nullptr) {
        resolve(stmt->value.get());
    }
}

void Resolver::visitSwitchStmt(SwitchStmt* stmt) {
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