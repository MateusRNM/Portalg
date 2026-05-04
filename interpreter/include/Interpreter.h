#pragma once
#include "Expr.h"
#include "Stmt.h"
#include "Environment.h"
#include <any>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <cmath>
#include <iostream>

class Interpreter;

class PortalgCallable {
    public:
        virtual ~PortalgCallable() = default;
        virtual int arity() = 0;
        virtual std::any call(Interpreter* interpreter, const std::vector<std::any>& arguments) = 0;
};

class Interpreter : public ExprVisitor, public StmtVisitor {
private:
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();

    std::any evaluate(Expr* expr) {
        return expr->accept(this);
    }

    bool isReal(const std::any& operand) {
        return operand.type() == typeid(double);
    }

    bool isInteger(const std::any& operand) {
        return operand.type() == typeid(long long);
    }

    double getAsDouble(const std::any& operand) {
        if(isReal(operand)) return std::any_cast<double>(operand);
        if(isInteger(operand)) return (double)std::any_cast<long long>(operand);
        throw std::runtime_error("Falha: valor não é numérico.");
    }

    bool isEqual(const std::any& a, const std::any& b) {
        if(a.type() == b.type()) {
            if(a.type() == typeid(std::string)) return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
            if(a.type() == typeid(bool)) return std::any_cast<bool>(a) == std::any_cast<bool>(b);
            if(isInteger(a)) return std::any_cast<long long>(a) == std::any_cast<long long>(b);
            if(isReal(a)) return std::any_cast<double>(a) == std::any_cast<double>(b);
        }
        
        if((isInteger(a) || isReal(a)) && (isInteger(b) || isReal(b))) {
            return getAsDouble(a) == getAsDouble(b);
        }

        return false;
    }


public:
    Interpreter() {

        struct NativeEscreva : public PortalgCallable {
            int arity() override { return -1; }
            std::any call(Interpreter* interpreter, const std::vector<std::any>& arguments) override {
                for(const auto& arg: arguments) {
                    std::cout << interpreter->stringify(arg);
                }
                return {};
            }
        };

        struct NativeEscreval : public PortalgCallable {
            int arity() override { return -1; }
            std::any call(Interpreter* interpreter, const std::vector<std::any>& arguments) override {
                for(const auto& arg: arguments) {
                    std::cout << interpreter->stringify(arg);
                }
                std::cout << '\n';
                return {};
            }
        };

        struct NativeLeia : public PortalgCallable {
            int arity() override { return 0; }
            std::any call(Interpreter* interpreter, const std::vector<std::any>& arguments) override {
                std::string input;
                std::getline(std::cin, input);
                return input;
            }
        };

        environment->define("escreva", std::make_shared<PortalgCallable*>(new NativeEscreva()), true, {});
        environment->define("escreval", std::make_shared<PortalgCallable*>(new NativeEscreval()), true, {});
        environment->define("leia", std::make_shared<PortalgCallable*>(new NativeLeia()), true, {});
    }

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
        try {
            for (const auto& statement : statements) {
                execute(statement.get());
            }
        } catch (const RuntimeError& error) {
            std::cerr << "Erro de Execução (Linha " << error.token.line << "): " << error.what() << "\n";
        }
    }

    void execute(Stmt* stmt) {
        stmt->accept(this);
    }

    std::string stringify(const std::any& operand) {
        if(operand.type() == typeid(std::string)) {
            return std::any_cast<std::string>(operand);
        }

        if(operand.type() == typeid(char)) {
            return std::string(1, std::any_cast<char>(operand)); 
        }

        if(isInteger(operand)) {
            return std::to_string(std::any_cast<long long>(operand));
        }

        if(isReal(operand)) {
            std::string str = std::to_string(std::any_cast<double>(operand));
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') str.pop_back();
            return str;
        }

        if(operand.type() == typeid(bool)) {
            return std::any_cast<bool>(operand) ? "verdadeiro" : "falso";
        }

        return "Impossível converter para texto.";
    }

    std::any visitLiteralExpr(LiteralExpr* expr) override {
        return expr->value;
    }

    std::any visitBinaryExpr(BinaryExpr* expr) override {
        std::any left = evaluate(expr->left.get());
        std::any right = evaluate(expr->right.get());

        switch(expr->op.type) {
            case TokenType::MINUS:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Tentativa de subtração com valores não numéricos.");
                }
                
                if(isInteger(left) && isInteger(right)) {
                    return std::any_cast<long long>(left) - std::any_cast<long long>(right);
                }
  
                return getAsDouble(left) - getAsDouble(right);

            case TokenType::STAR:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Tentativa de multiplicação com valores não numéricos.");
                }

                if(isInteger(left) && isInteger(right)) {
                    return std::any_cast<long long>(left) * std::any_cast<long long>(right);
                }

                return getAsDouble(left) * getAsDouble(right);

            case TokenType::SLASH:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Tentativa de divisão com valores não numéricos.");
                }

                if(getAsDouble(right) == 0.0) {
                    throw RuntimeError(expr->op, "Tentativa de divisão por zero.");
                }

                return getAsDouble(left) / getAsDouble(right);

            case TokenType::PLUS:
                if(left.type() == typeid(std::string) || left.type() == typeid(char) || right.type() == typeid(std::string) || right.type() == typeid(char)) {
                    return stringify(left) + stringify(right);
                }
                
                if((isReal(left) || isInteger(left)) && (isReal(right) || isInteger(right))) {
                    if(isInteger(left) && isInteger(right)) {
                        return std::any_cast<long long>(left) + std::any_cast<long long>(right);
                    }
                    return getAsDouble(left) + getAsDouble(right);
                }
                
                throw RuntimeError(expr->op, "Operandos inválidos para o operador '+'.");

            case TokenType::MOD:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Tentativa de resto da divisão com valores não numéricos.");
                }

                if(getAsDouble(right) == 0.0) {
                    throw RuntimeError(expr->op, "Tentativa de divisão por zero.");
                }

                if(isInteger(left) && isInteger(right)) {
                    return std::any_cast<long long>(left) % std::any_cast<long long>(right);
                }

                return std::fmod(getAsDouble(left), getAsDouble(right));
            
            case TokenType::POTENCY: {
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Tentativa de exponenciação com valores não numéricos.");
                }

                double base = getAsDouble(left);
                double exponent = getAsDouble(right);

                if(isInteger(left) && isInteger(right) && std::any_cast<long long>(right) >= 0) {
                    return (long long)std::pow(base, exponent);
                }

                return std::pow(base, exponent);
            }

            case TokenType::EQUAL_EQUAL:
                return isEqual(left, right);
            
            case TokenType::NOT_EQUAL:
                return !isEqual(left, right);
            
            case TokenType::GREATER:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Operador > deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) > getAsDouble(right);
            
            case TokenType::GREATER_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Operador >= deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) >= getAsDouble(right);

            case TokenType::LESS:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Operador < deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) < getAsDouble(right);

            case TokenType::LESS_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(expr->op, "Operador <= deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) <= getAsDouble(right);

            default:
                break;
        }

        return {}; 
    }

    std::any visitUnaryExpr(UnaryExpr* expr) override {
        std::any right = evaluate(expr->right.get());
        switch(expr->op.type) {
            case TokenType::MINUS:
                if(isReal(right)) {
                    return -std::any_cast<double>(right);
                }

                if(isInteger(right)) {
                    return -std::any_cast<long long>(right);
                }

                throw RuntimeError(expr->op, "O operador unário '-' exige um valor numérico.");

            case TokenType::NOT:
                if(right.type() == typeid(bool)) {
                    return !std::any_cast<bool>(right);
                }

                throw RuntimeError(expr->op, "O operador de negação exige um valor lógico.");

            default:
                break;
        }
        return {};
    }

    std::any visitPrefixPostfixExpr(PrefixPostfixExpr* expr) override {
        if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr->target.get())) {
            std::any currentValue = environment->get(varExpr->name);

            if(!isReal(currentValue) && !isInteger(currentValue)) {
                throw RuntimeError(expr->op, "Operadores de incremento e decremento só podem ser aplicados a variáveis numéricas.");
            }

            std::any newValue;
            if(isInteger(currentValue)) {
                long long val = std::any_cast<long long>(currentValue);
                newValue = (expr->op.type == TokenType::PLUS_PLUS) ? (val + 1) : (val - 1);
            } else {
                double val = std::any_cast<double>(currentValue);
                newValue = (expr->op.type == TokenType::PLUS_PLUS) ? (val + 1.0) : (val - 1.0);
            }

            environment->assign(varExpr->name, newValue);

            if(expr->isPrefix) {
                return newValue;
            } else {
                return currentValue;
            }
        } else if(IndexAccessExpr* indexExpr = dynamic_cast<IndexAccessExpr*>(expr->target.get())) {
            // Implementar incremento em índices de vetores 
        }
        throw RuntimeError(expr->op, "Alvo inválido para incremento/decremento.");
    }

    std::any visitVariableExpr(VariableExpr* expr) override {
        return environment->get(expr->name);
    }

    std::any visitLogicalExpr(LogicalExpr* expr) override {
        std::any left = evaluate(expr->left.get());
        if(left.type() != typeid(bool)) {
            throw RuntimeError(expr->op, "Tentativa de usar operador lógico com valores não lógicos.");
        }

        bool leftVal = std::any_cast<bool>(left);

        if(expr->op.type == TokenType::OR) {
            if(leftVal) return true;
        } else {
            if(!leftVal) return false;
        }

        std::any right = evaluate(expr->right.get());
        if(right.type() != typeid(bool)) {
            throw RuntimeError(expr->op, "Tentativa de usar operador lógico com valores não lógicos.");
        }

        return std::any_cast<bool>(right);
    }

    std::any visitTernaryExpr(TernaryExpr* expr) override {
        std::any condition = evaluate(expr->condition.get());
        if(condition.type() != typeid(bool)) {
            throw RuntimeError(expr->query, "A condição do operador ternário deve ser um valor lógico.");
        }

        if(std::any_cast<bool>(condition)) {
            return evaluate(expr->trueExpr.get());
        } else {
            return evaluate(expr->falseExpr.get());
        }
    }

    void visitExpressionStmt(ExpressionStmt* stmt) override {
        evaluate(stmt->expression.get());
    }

    void visitVarDeclStmt(VarDeclStmt* stmt) override {
        std::any value = evaluate(stmt->initializer.get());

        if(value.type() == typeid(std::string) && stmt->type.size() == 1) {
            std::string textValue = std::any_cast<std::string>(value);
            try {
                if (stmt->type[0].type == TokenType::KW_INTEGER) {
                    value = std::stoll(textValue);
                } else if (stmt->type[0].type == TokenType::KW_REAL) {
                    value = std::stod(textValue);
                }
            } catch (const std::exception& e) {
                throw RuntimeError(stmt->name, "Não foi possível converter o texto '" + textValue + "' para um número válido.");
            }
        }

        if(stmt->type.size() == 1 && stmt->type[0].type == TokenType::KW_INTEGER) {
            if(isReal(value)) {
                value = (long long)std::any_cast<double>(value);
            }
        }

        environment->define(stmt->name.lexeme, std::move(value), stmt->isConst, stmt->type);
    }

    std::any visitAssignExpr(AssignExpr* expr) override {
        std::any value = evaluate(expr->value.get());
        std::vector<Token> varType = environment->getType(expr->name);

        if(value.type() == typeid(std::string) && varType.size() == 1) {
            std::string textValue = std::any_cast<std::string>(value);
            try {
                if(varType[0].type == TokenType::KW_INTEGER) {
                    value = std::stoll(textValue);
                } else if(varType[0].type == TokenType::KW_REAL) {
                    value = std::stod(textValue);
                }
            } catch (const std::exception& e) {
                throw RuntimeError(expr->name, "Não foi possível converter o texto '" + textValue + "' para um número válido.");
            }
        }

        if(varType.size() == 1 && varType[0].type == TokenType::KW_INTEGER) {
            if (isReal(value)) {
                value = (long long)std::any_cast<double>(value);
            }
        }

        environment->assign(expr->name, value);
        return value;
    }

    std::any visitCallExpr(CallExpr* expr) override {
        std::any callee = evaluate(expr->callee.get());

        std::vector<std::any> arguments;
        for(const auto& argExpr : expr->arguments) {
            arguments.emplace_back(evaluate(argExpr.get()));
        }

        if(callee.type() != typeid(std::shared_ptr<PortalgCallable*>)) {
            throw RuntimeError(expr->openToken, "Só é permitido chamar funções e métodos.");
        }

        PortalgCallable* function = *std::any_cast<std::shared_ptr<PortalgCallable*>>(callee);

        if(function->arity() != -1 && arguments.size() != function->arity()) {
            throw RuntimeError(expr->openToken, "A função esperava " + std::to_string(function->arity()) + " argumentos, mas recebeu " + std::to_string(arguments.size()) + ".");
        }

        return function->call(this, arguments);
    }

    std::any visitMethodCallExpr(MethodCallExpr* expr) override {
        throw std::runtime_error("visitMethodCallExpr não implementado ainda.");
    }

    std::any visitIndexAccessExpr(IndexAccessExpr* expr) override {
        throw std::runtime_error("visitIndexAccessExpr não implementado ainda.");
    }

    std::any visitIndexAssignExpr(IndexAssignExpr* expr) override {
        throw std::runtime_error("visitIndexAssignExpr não implementado ainda.");
    }

    std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr) override {
        throw std::runtime_error("visitArrayLiteralExpr não implementado ainda.");
    }

    std::any visitInstantiateExpr(InstantiateExpr* expr) override {
        throw std::runtime_error("visitInstantiateExpr não implementado ainda.");
    }

    void visitBlockStmt(BlockStmt* stmt) override {
        throw std::runtime_error("visitBlockStmt não implementado ainda.");
    }

    void visitIfStmt(IfStmt* stmt) override {
        throw std::runtime_error("visitIfStmt não implementado ainda.");
    }

    void visitWhileStmt(WhileStmt* stmt) override {
        throw std::runtime_error("visitWhileStmt não implementado ainda.");
    }

    void visitForStmt(ForStmt* stmt) override {
        throw std::runtime_error("visitForStmt não implementado ainda.");
    }

    void visitBreakStmt(BreakStmt* stmt) override {
        throw std::runtime_error("visitBreakStmt não implementado ainda.");
    }

    void visitContinueStmt(ContinueStmt* stmt) override {
        throw std::runtime_error("visitContinueStmt não implementado ainda.");
    }

    void visitReturnStmt(ReturnStmt* stmt) override {
        throw std::runtime_error("visitReturnStmt não implementado ainda.");
    }

    void visitFunctionStmt(FunctionStmt* stmt) override {
        throw std::runtime_error("visitFunctionStmt não implementado ainda.");
    }

    void visitSwitchStmt(SwitchStmt* stmt) override {
        throw std::runtime_error("visitSwitchStmt não implementado ainda.");
    }
};