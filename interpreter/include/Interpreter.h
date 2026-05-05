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

    std::any calculateMathAndRelationals(const std::any& left, Token op, const std::any& right) {
        switch(op.type) {
            case TokenType::MINUS:
            case TokenType::MINUS_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Tentativa de subtração com valores não numéricos.");
                }
                
                if(isInteger(left) && isInteger(right)) {
                    return std::any_cast<long long>(left) - std::any_cast<long long>(right);
                }
  
                return getAsDouble(left) - getAsDouble(right);

            case TokenType::STAR:
            case TokenType::STAR_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Tentativa de multiplicação com valores não numéricos.");
                }

                if(isInteger(left) && isInteger(right)) {
                    return std::any_cast<long long>(left) * std::any_cast<long long>(right);
                }

                return getAsDouble(left) * getAsDouble(right);

            case TokenType::SLASH:
            case TokenType::SLASH_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Tentativa de divisão com valores não numéricos.");
                }

                if(getAsDouble(right) == 0.0) {
                    throw RuntimeError(op, "Tentativa de divisão por zero.");
                }

                return getAsDouble(left) / getAsDouble(right);

            case TokenType::PLUS:
            case TokenType::PLUS_EQUAL:
                if(left.type() == typeid(std::string) || left.type() == typeid(char) || right.type() == typeid(std::string) || right.type() == typeid(char)) {
                    return stringify(left) + stringify(right);
                }
                
                if((isReal(left) || isInteger(left)) && (isReal(right) || isInteger(right))) {
                    if(isInteger(left) && isInteger(right)) {
                        return std::any_cast<long long>(left) + std::any_cast<long long>(right);
                    }
                    return getAsDouble(left) + getAsDouble(right);
                }
                
                throw RuntimeError(op, "Operandos inválidos para o operador '+'.");

            case TokenType::MOD:
            case TokenType::MOD_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Tentativa de resto da divisão com valores não numéricos.");
                }

                if(getAsDouble(right) == 0.0) {
                    throw RuntimeError(op, "Tentativa de divisão por zero.");
                }

                if(isInteger(left) && isInteger(right)) {
                    return std::any_cast<long long>(left) % std::any_cast<long long>(right);
                }

                return std::fmod(getAsDouble(left), getAsDouble(right));
            
            case TokenType::POTENCY:
            case TokenType::POTENCY_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Tentativa de exponenciação com valores não numéricos.");
                }

                if(isInteger(left) && isInteger(right) && std::any_cast<long long>(right) >= 0) {
                    return (long long)std::pow(getAsDouble(left), getAsDouble(right));
                }

                return std::pow(getAsDouble(left), getAsDouble(right));
            

            case TokenType::EQUAL_EQUAL:
                return isEqual(left, right);
            
            case TokenType::NOT_EQUAL:
                return !isEqual(left, right);
            
            case TokenType::GREATER:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Operador > deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) > getAsDouble(right);
            
            case TokenType::GREATER_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Operador >= deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) >= getAsDouble(right);

            case TokenType::LESS:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Operador < deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) < getAsDouble(right);

            case TokenType::LESS_EQUAL:
                if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                    throw RuntimeError(op, "Operador <= deve ser usado apenas com valores numéricos.");
                }
                return getAsDouble(left) <= getAsDouble(right);

            default:
                break;
        }
        return {};
    }

    bool matchType(std::any& value, const std::vector<Token>& typeTokens, size_t& index) {
        if (index >= typeTokens.size()) return false;

        TokenType t = typeTokens[index++].type;

        if (t == TokenType::KW_INTEGER && isReal(value)) {
            value = (long long)std::any_cast<double>(value);
        } else if (t == TokenType::KW_REAL && isInteger(value)) {
            value = (double)std::any_cast<long long>(value);
        }

        if(t == TokenType::KW_INTEGER) return value.type() == typeid(long long);
        if(t == TokenType::KW_REAL) return value.type() == typeid(double);
        if(t == TokenType::KW_TEXT) return value.type() == typeid(std::string);
        if(t == TokenType::KW_LOGIC) return value.type() == typeid(bool);
        if(t == TokenType::KW_CHAR) return value.type() == typeid(char);
        
        if(t == TokenType::KW_VECTOR) {
            if(index < typeTokens.size() && typeTokens[index].type == TokenType::LESS) {
                index++;
                size_t innerTypeStartIndex = index;
            
                if(value.type() != typeid(TypedArray)) return false;
                
                TypedArray typedArray = std::any_cast<TypedArray>(value);

                if(!typedArray.typeTokens->empty()) {
                    std::vector<Token> expectedFullType;
                    expectedFullType.push_back({TokenType::KW_VECTOR, "vetor", 0});
                    expectedFullType.push_back({TokenType::LESS, "<", 0});
                    
                    size_t tempIdx = innerTypeStartIndex;
                    int depth = 1;
                    while(tempIdx < typeTokens.size() && depth > 0) {
                        expectedFullType.push_back(typeTokens[tempIdx]);
                        if(typeTokens[tempIdx].type == TokenType::LESS) depth++;
                        else if(typeTokens[tempIdx].type == TokenType::GREATER) depth--;
                        tempIdx++;
                    }

                    if(expectedFullType.size() != typedArray.typeTokens->size()) return false;
                    for(size_t i = 0; i < expectedFullType.size(); i++) {
                        if(expectedFullType[i].type != (*typedArray.typeTokens)[i].type) return false;
                    }
                    
                    index = tempIdx;
                    return true;
                }

                std::vector<std::any>& vec = *(typedArray.elements);
                
                if(vec.empty()) {
                    int openBrackets = 1;
                    while(index < typeTokens.size() && openBrackets > 0) {
                        if (typeTokens[index].type == TokenType::LESS) openBrackets++;
                        else if (typeTokens[index].type == TokenType::GREATER) openBrackets--;
                        index++;
                    }
                    return true;
                }
                
                for(size_t i = 0; i < vec.size(); i++) {
                    size_t tempIndex = innerTypeStartIndex;
                    
                    if(!matchType(vec[i], typeTokens, tempIndex)) {
                        return false;
                    }
                    
                    if(i == vec.size() - 1) {
                        index = tempIndex; 
                    }
                }

                value = typedArray;
                
                if(index < typeTokens.size() && typeTokens[index].type == TokenType::GREATER) {
                    index++; 
                }
                
                return true;
            }
        }
        return false;
    }

    std::vector<Token> extractSubtype(const std::vector<Token>& fullType) {
        if(fullType.empty() || fullType[0].type != TokenType::KW_VECTOR) {
            return fullType;
        }

        std::vector<Token> subType;
        int depth = 0;
        for(size_t i = 2; i < fullType.size(); i++) {
            if(fullType[i].type == TokenType::LESS) depth++;
            else if(fullType[i].type == TokenType::GREATER) {
                if(depth == 0) break;
                depth--;
            }
            subType.emplace_back(fullType[i]);
        }

        return subType;
    }

    void validateType(const std::vector<Token>& typeTokens, std::any& value, Token errorToken) {
        if(typeTokens.empty()) return;
        size_t index = 0;
        if(!matchType(value, typeTokens, index)) {
            std::string typeName = "";
            for(const auto& t : typeTokens) {   
                typeName += t.lexeme;
            }
            throw RuntimeError(errorToken, "Valor incompatível. Esperado o tipo '" + typeName + "'.");
        }
    }

    void enforceType(const std::vector<Token>& typeTokens, std::any& value, Token errorToken) {
        if(typeTokens.empty()) return;

        if(value.type() == typeid(std::string) && typeTokens.size() == 1) {
            std::string textValue = std::any_cast<std::string>(value);
            try {
                if (typeTokens[0].type == TokenType::KW_INTEGER) value = std::stoll(textValue);
                else if (typeTokens[0].type == TokenType::KW_REAL) value = std::stod(textValue);
            } catch (...) {}
        }

        validateType(typeTokens, value, errorToken);
    }

    std::any applyIncrementDecrement(const std::any& currentValue, TokenType opType, Token errorToken) {
        if (!isReal(currentValue) && !isInteger(currentValue)) {
            throw RuntimeError(errorToken, "Operadores de incremento e decremento só podem ser aplicados a valores numéricos.");
        }
        if (isInteger(currentValue)) {
            long long val = std::any_cast<long long>(currentValue);
            return (opType == TokenType::PLUS_PLUS) ? (val + 1) : (val - 1);
        } else {
            double val = std::any_cast<double>(currentValue);
            return (opType == TokenType::PLUS_PLUS) ? (val + 1.0) : (val - 1.0);
        }
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

        environment->define("escreva", std::shared_ptr<PortalgCallable>(std::make_shared<NativeEscreva>()), true, {});
        environment->define("escreval", std::shared_ptr<PortalgCallable>(std::make_shared<NativeEscreval>()), true, {});
        environment->define("leia", std::shared_ptr<PortalgCallable>(std::make_shared<NativeLeia>()), true, {});
    }

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
        loopDepth = 0;
        switchDepth = 0;
        functionDepth = 0;
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

        if(operand.type() == typeid(TypedArray)) {
            std::string result = "[";
            const auto& elements = *(std::any_cast<TypedArray>(operand).elements);
            for(size_t i = 0; i < elements.size(); i++) {
                result += stringify(elements[i]);
                if(i < elements.size()-1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        }

        return "Impossível converter para texto.";
    }

    std::any visitLiteralExpr(LiteralExpr* expr) override {
        return expr->value;
    }

    std::any visitBinaryExpr(BinaryExpr* expr) override {
        std::any left = evaluate(expr->left.get());
        std::any right = evaluate(expr->right.get());
        return calculateMathAndRelationals(left, expr->op, right);
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
            std::any newValue = applyIncrementDecrement(currentValue, expr->op.type, expr->op);

            environment->assign(varExpr->name, newValue);

            if(expr->isPrefix) {
                return newValue;
            } else {
                return currentValue;
            }
        } else if(IndexAccessExpr* indexExpr = dynamic_cast<IndexAccessExpr*>(expr->target.get())) {
            std::vector<long long> indexes;
            Expr* currentTarget = indexExpr->target.get();
            
            std::any evaluatedIndex = evaluate(indexExpr->index.get());
            if(!isInteger(evaluatedIndex)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
            indexes.push_back(std::any_cast<long long>(evaluatedIndex));

            while(IndexAccessExpr* idxAccess = dynamic_cast<IndexAccessExpr*>(currentTarget)) {
                std::any idxVal = evaluate(idxAccess->index.get());
                if(!isInteger(idxVal)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
                indexes.insert(indexes.begin(), std::any_cast<long long>(idxVal));
                currentTarget = idxAccess->target.get();
            }

            VariableExpr* varExpr = dynamic_cast<VariableExpr*>(currentTarget);
            if(!varExpr) throw RuntimeError(expr->op, "O alvo da atribuição de índice deve ser uma variável.");

            std::any targetObj = environment->get(varExpr->name);
            std::any* currentAny = &targetObj;
            
            for(size_t i = 0; i < indexes.size(); i++) {
                long long idx = indexes[i];
                
                if(currentAny->type() == typeid(TypedArray)) {
                    TypedArray* arrayPtr = std::any_cast<TypedArray>(currentAny);
                    if(idx < 0 || idx >= arrayPtr->elements->size()) throw RuntimeError(expr->op, "Índice do vetor fora dos limites.");
                    
                    if(i == indexes.size() - 1) {
                        std::any& finalElement = (*arrayPtr->elements)[idx];
                        std::any currentValue = finalElement;
                        std::any newValue = applyIncrementDecrement(currentValue, expr->op.type, expr->op);

                        finalElement = newValue;
                        return expr->isPrefix ? newValue : currentValue;
                    } else {
                        currentAny = &((*arrayPtr->elements)[idx]);
                    }
                } else {
                    throw RuntimeError(expr->op, "Tentativa de acessar índice de um tipo não indexável.");
                }
            }
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
        enforceType(stmt->type, value, stmt->name);
        environment->define(stmt->name.lexeme, std::move(value), stmt->isConst, stmt->type);
    }

    std::any visitAssignExpr(AssignExpr* expr) override {
        std::any value = evaluate(expr->value.get());
        if(expr->op.type != TokenType::EQUAL) {
            std::any currentValue = environment->get(expr->name);
            value = calculateMathAndRelationals(currentValue, expr->op, value);
        }

        std::vector<Token> varType = environment->getType(expr->name);
        enforceType(varType, value, expr->name);

        environment->assign(expr->name, value);
        return value;
    }

    std::any visitCallExpr(CallExpr* expr) override {
        std::any callee = evaluate(expr->callee.get());

        std::vector<std::any> arguments;
        for(const auto& argExpr : expr->arguments) {
            arguments.emplace_back(evaluate(argExpr.get()));
        }

        if(callee.type() != typeid(std::shared_ptr<PortalgCallable>)) {
            throw RuntimeError(expr->openToken, "Só é permitido chamar funções e métodos.");
        }

        std::shared_ptr<PortalgCallable> function = std::any_cast<std::shared_ptr<PortalgCallable>>(callee);

        if(function->arity() != -1 && arguments.size() != function->arity()) {
            throw RuntimeError(expr->openToken, "A função esperava " + std::to_string(function->arity()) + " argumentos, mas recebeu " + std::to_string(arguments.size()) + ".");
        }

        return function->call(this, arguments);
    }

    std::any visitMethodCallExpr(MethodCallExpr* expr) override {
        throw std::runtime_error("visitMethodCallExpr não implementado ainda.");
    }

    std::any visitIndexAccessExpr(IndexAccessExpr* expr) override {
        std::any target = evaluate(expr->target.get());
        std::any indexValue = evaluate(expr->index.get());

        if(!isInteger(indexValue)) {
            throw RuntimeError(expr->bracketToken, "O índice deve ser um número inteiro positivo.");
        }

        long long index = std::any_cast<long long>(indexValue);

        if(target.type() == typeid(std::string)) {
            std::string text = std::any_cast<std::string>(target);
            if(index < 0 || index >= text.length()) {
                throw RuntimeError(expr->bracketToken, "Índice de texto fora dos limites.");
            }
            return text[index];
        }

        if(target.type() == typeid(TypedArray)) {
            const TypedArray& typedArray = std::any_cast<const TypedArray&>(target);
            if(index < 0 || index >= typedArray.elements->size()) {
                throw RuntimeError(expr->bracketToken, "Índice do vetor fora dos limites.");
            }
            return (*typedArray.elements)[index];
        }

        throw RuntimeError(expr->bracketToken, "Só é possível acessar índices de vetores ou textos.");
    }

    std::any visitIndexAssignExpr(IndexAssignExpr* expr) override {
        std::vector<long long> indexes;
        Expr* currentTarget = expr->target.get();
        
        std::any evaluatedIndex = evaluate(expr->index.get());
        if(!isInteger(evaluatedIndex)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
        indexes.push_back(std::any_cast<long long>(evaluatedIndex));

        while(IndexAccessExpr* idxAccess = dynamic_cast<IndexAccessExpr*>(currentTarget)) {
            std::any idxVal = evaluate(idxAccess->index.get());
            if(!isInteger(idxVal)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
            indexes.insert(indexes.begin(), std::any_cast<long long>(idxVal));
            currentTarget = idxAccess->target.get();
        }

        VariableExpr* varExpr = dynamic_cast<VariableExpr*>(currentTarget);
        if(!varExpr) throw RuntimeError(expr->op, "O alvo da atribuição de índice deve ser uma variável.");

        std::any targetObj = environment->get(varExpr->name);
        std::any newValue = evaluate(expr->value.get());
        std::any* currentAny = &targetObj;
        
        for(size_t i = 0; i < indexes.size(); i++) {
            long long idx = indexes[i];
            
            if(currentAny->type() == typeid(TypedArray)) {
                TypedArray* arrayPtr = std::any_cast<TypedArray>(currentAny);
                if(idx < 0 || idx >= arrayPtr->elements->size()) throw RuntimeError(expr->op, "Índice do vetor fora dos limites.");
                
                if(i == indexes.size() - 1) {
                    std::any& finalElement = (*arrayPtr->elements)[idx];
                    
                    if(expr->op.type != TokenType::EQUAL) {
                        newValue = calculateMathAndRelationals(finalElement, expr->op, newValue);
                    }

                    std::vector<Token> currentExpectedType = environment->getType(varExpr->name);
                    for(size_t d = 0; d <= i; d++) {
                        currentExpectedType = extractSubtype(currentExpectedType);
                    }
                    enforceType(currentExpectedType, newValue, expr->op);
                    
                    finalElement = newValue;
                    return newValue;
                    
                } else {
                    currentAny = &((*arrayPtr->elements)[idx]);
                }
            } else if(currentAny->type() == typeid(std::string)) {
                if(i != indexes.size() - 1) throw RuntimeError(expr->op, "Não é possível acessar índices dentro de um texto.");
                
                std::string* strPtr = std::any_cast<std::string>(currentAny);
                if(idx < 0 || idx >= strPtr->length()) throw RuntimeError(expr->op, "Índice de texto fora dos limites.");
                
                if(expr->op.type != TokenType::EQUAL) {
                    std::any currentChar = std::string(1, (*strPtr)[idx]);
                    newValue = calculateMathAndRelationals(currentChar, expr->op, newValue);
                }

                if(newValue.type() == typeid(char)) {
                    (*strPtr)[idx] = std::any_cast<char>(newValue);
                } else if(newValue.type() == typeid(std::string) && std::any_cast<std::string>(newValue).length() == 1) {
                    (*strPtr)[idx] = std::any_cast<std::string>(newValue)[0];
                } else {
                    throw RuntimeError(expr->op, "Apenas caracteres podem ser atribuídos a índices de texto.");
                }
                
                environment->assign(varExpr->name, targetObj);
                return newValue;
            } else {
                throw RuntimeError(expr->op, "Tentativa de acessar índice de um tipo não indexável.");
            }
        }
        return newValue;
    }

    std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr) override {
        std::vector<std::any> elements;
        for(const auto& elementExpr : expr->elements) {
            elements.emplace_back(evaluate(elementExpr.get()));
        }
        return TypedArray{std::make_shared<std::vector<Token>>(), std::make_shared<std::vector<std::any>>(elements)};
    }

    std::any visitInstantiateExpr(InstantiateExpr* expr) override {
        std::any sizeValue = evaluate(expr->size.get());
        if(!isInteger(sizeValue)) {
            throw RuntimeError(expr->type[0], "O tamanho do vetor deve ser um número inteiro positivo.");
        }

        long long size = std::any_cast<long long>(sizeValue);
        if(size < 0) {
            throw RuntimeError(expr->type[0], "O tamanho do vetor deve ser positivo.");
        }

        std::any initialValue = evaluate(expr->initialValue.get());

        std::vector<Token> expectedSubType = extractSubtype(expr->type);
        validateType(expectedSubType, initialValue, expr->type[0]);

        std::vector<std::any> elements(size, initialValue);
        return TypedArray{std::make_shared<std::vector<Token>>(expr->type), std::make_shared<std::vector<std::any>>(elements)};
    }

    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> innerEnvironment) {
        std::shared_ptr<Environment> previousEnv = this->environment;
        try {
            this->environment = innerEnvironment;

            for(const auto& st : statements) {
                execute(st.get());
            }

            this->environment = previousEnv;
        } catch(...) {
            this->environment = previousEnv;
            throw;
        }
    }

    void visitBlockStmt(BlockStmt* stmt) override {
        executeBlock(stmt->statements, std::make_shared<Environment>(this->environment));
    }

    void visitIfStmt(IfStmt* stmt) override {
        std::any conditionResult = evaluate(stmt->condition.get());
        if(conditionResult.type() != typeid(bool)) {
            throw RuntimeError(stmt->ifToken, "A condição do 'se' deve retornar um valor lógico.");
        }

        if(std::any_cast<bool>(conditionResult)) {
            execute(stmt->thenBranch.get());
        } else if(stmt->elseBranch != nullptr) {
            execute(stmt->elseBranch.get());
        }
    }

    void visitWhileStmt(WhileStmt* stmt) override {
        loopDepth++;
        try {
            while(true) {
                std::any conditionResult = evaluate(stmt->condition.get());

                if(conditionResult.type() != typeid(bool)) {
                    throw RuntimeError(stmt->keyword, "A condição do laço de repetição deve ser um valor lógico.");
                }

                if(!std::any_cast<bool>(conditionResult)) {
                    break;
                }

                try {
                    execute(stmt->body.get());
                } catch(const ContinueException& e) {
                }
            }
        } catch(const BreakException& e) {
        }
        loopDepth--;
    }

    void visitForStmt(ForStmt* stmt) override {
        std::shared_ptr<Environment> previousEnv = this->environment;
        this->environment = std::make_shared<Environment>(this->environment);
        loopDepth++;
        try {
            if(stmt->initializer != nullptr) {
                if(BlockStmt* blockInit = dynamic_cast<BlockStmt*>(stmt->initializer.get())) {
                    for(const auto& st : blockInit->statements) {
                        execute(st.get());
                    }
                }
                execute(stmt->initializer.get());
            }

            while(true) {
                if(stmt->condition != nullptr) {
                    std::any conditionResult = evaluate(stmt->condition.get());
                    
                    if(conditionResult.type() != typeid(bool)) {
                        throw RuntimeError(stmt->keyword, "A condição do laço de repetição deve ser um valor lógico.");
                    }

                    if(!std::any_cast<bool>(conditionResult)) {
                        break;
                    }
                }

                try {
                    execute(stmt->body.get());
                } catch(const ContinueException& e) {
                }

                if(stmt->increment != nullptr) {
                    evaluate(stmt->increment.get());
                }
            }
        } catch(const BreakException& e) {
        } catch(...) {
            this->environment = previousEnv;
            loopDepth--;
            throw;
        }
        loopDepth--;
        this->environment = previousEnv;
    }

    void visitBreakStmt(BreakStmt* stmt) override {
        if(loopDepth == 0 && switchDepth == 0) {
            throw RuntimeError(stmt->keyword, "O comando 'parar' só pode ser utilizado dentro de laços de repetição ou bloco 'escolha'.");
        }
        throw BreakException();
    }

    void visitContinueStmt(ContinueStmt* stmt) override {
        if(loopDepth == 0) {
            throw RuntimeError(stmt->keyword, "O comando 'continuar' só pode ser utilizado dentro de laços de repetição.");
        }
        throw ContinueException();
    }

    void visitReturnStmt(ReturnStmt* stmt) override {
        if(functionDepth == 0) {
            throw RuntimeError(stmt->keyword, "O comando 'retornar' só pode ser utilizado dentro de funções.");
        }
        // throw ReturnException(value);
    }

    void visitFunctionStmt(FunctionStmt* stmt) override {
        throw std::runtime_error("visitFunctionStmt não implementado ainda.");
    }

    void visitSwitchStmt(SwitchStmt* stmt) override {
        std::any switchValue = evaluate(stmt->target.get());
        bool fallthrough = false;
        switchDepth++;
        try {
            for(const auto& switchCase : stmt->cases) {
                if(fallthrough) {
                    executeBlock(switchCase.body, std::make_shared<Environment>(this->environment));
                } else {
                    bool isDefault = (switchCase.matchExpr == nullptr);
                    bool matched;

                    if(isDefault) {
                        matched = true;
                    } else {
                        std::any caseValue = evaluate(switchCase.matchExpr.get());
                        matched = isEqual(switchValue, caseValue);
                    }

                    if(matched) {
                        fallthrough = true;
                        executeBlock(switchCase.body, std::make_shared<Environment>(this->environment));
                    }
                }
            }
        } catch(const BreakException& e) {
        }
        switchDepth--;
    }
};