#include "Interpreter.h"
#include "json.hpp"

using json = nlohmann::json;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_ASYNC_JS(void, debugger_pause, (const char* json_state, int actual_line), {
    return await new Promise((resolve) => {
        postMessage({
            type: "DEBUG_STATE",
            state: UTF8ToString(json_state),
            line: actual_line
        });

        const listener = (event) => {
            if(event.data.command === "DEBUG_NEXT_STEP") {
                self.removeEventListener("message", listener);
                resolve();
            }
        };
        self.addEventListener("message", listener);
    });
});

#endif

PortalgValue Interpreter::evaluate(Expr* expr) {
    return expr->accept(this);
}

bool Interpreter::isReal(const PortalgValue& operand) {
    return std::holds_alternative<double>(operand);
}

bool Interpreter::isInteger(const PortalgValue& operand) {
    return std::holds_alternative<long long>(operand);
}

double Interpreter::getAsDouble(const PortalgValue& operand) {
    if(isReal(operand)) return std::get<double>(operand);
    if(isInteger(operand)) return (double)std::get<long long>(operand);
    throw std::runtime_error("Falha: valor não é numérico.");
}

bool Interpreter::isEqual(const PortalgValue& a, const PortalgValue& b) {
    if(a.index() == b.index() && !std::holds_alternative<std::monostate>(a)) {
        if(std::holds_alternative<std::string>(a)) return std::get<std::string>(a) == std::get<std::string>(b);
        if(std::holds_alternative<bool>(a)) return std::get<bool>(a) == std::get<bool>(b);
        if(isInteger(a)) return std::get<long long>(a) == std::get<long long>(b);
        if(isReal(a)) return std::get<double>(a) == std::get<double>(b);
    }
    
    if((isInteger(a) || isReal(a)) && (isInteger(b) || isReal(b))) {
        return getAsDouble(a) == getAsDouble(b);
    }

    return false;
}

PortalgValue Interpreter::calculateMathAndRelationals(const PortalgValue& left, Token op, const PortalgValue& right) {
    switch(op.type) {
        case TokenType::MINUS:
        case TokenType::MINUS_EQUAL:
            if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                throw RuntimeError(op, "Tentativa de subtração com valores não numéricos.");
            }
            
            if(isInteger(left) && isInteger(right)) {
                return std::get<long long>(left) - std::get<long long>(right);
            }

            return getAsDouble(left) - getAsDouble(right);

        case TokenType::STAR:
        case TokenType::STAR_EQUAL:
            if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                throw RuntimeError(op, "Tentativa de multiplicação com valores não numéricos.");
            }

            if(isInteger(left) && isInteger(right)) {
                return std::get<long long>(left) * std::get<long long>(right);
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
            if(std::holds_alternative<std::string>(left) || std::holds_alternative<char>(left) || 
               std::holds_alternative<std::string>(right) || std::holds_alternative<char>(right)) {
                return stringify(left) + stringify(right);
            }
            
            if((isReal(left) || isInteger(left)) && (isReal(right) || isInteger(right))) {
                if(isInteger(left) && isInteger(right)) {
                    return std::get<long long>(left) + std::get<long long>(right);
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
                return std::get<long long>(left) % std::get<long long>(right);
            }

            return std::fmod(getAsDouble(left), getAsDouble(right));
        
        case TokenType::POTENCY:
        case TokenType::POTENCY_EQUAL:
            if((!isReal(left) && !isInteger(left)) || (!isReal(right) && !isInteger(right))) {
                throw RuntimeError(op, "Tentativa de exponenciação com valores não numéricos.");
            }

            if(isInteger(left) && isInteger(right) && std::get<long long>(right) >= 0) {
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
    return std::monostate{};
}

bool Interpreter::matchType(PortalgValue& value, const std::vector<Token>& typeTokens, size_t& index) {
    if (index >= typeTokens.size()) return false;

    TokenType t = typeTokens[index++].type;

    if (t == TokenType::KW_INTEGER && isReal(value)) {
        value = (long long)std::get<double>(value);
    } else if (t == TokenType::KW_REAL && isInteger(value)) {
        value = (double)std::get<long long>(value);
    }

    if(t == TokenType::KW_INTEGER) return std::holds_alternative<long long>(value);
    if(t == TokenType::KW_REAL) return std::holds_alternative<double>(value);
    if(t == TokenType::KW_TEXT) return std::holds_alternative<std::string>(value);
    if(t == TokenType::KW_LOGIC) return std::holds_alternative<bool>(value);
    if(t == TokenType::KW_CHAR) return std::holds_alternative<char>(value);
    
    if(t == TokenType::KW_VECTOR) {
        if(index < typeTokens.size() && typeTokens[index].type == TokenType::LESS) {
            index++;
            size_t innerTypeStartIndex = index;
        
            if(!std::holds_alternative<std::shared_ptr<TypedArray>>(value)) return false;
            
            auto typedArray = std::get<std::shared_ptr<TypedArray>>(value);

            if(!typedArray->typeTokens->empty()) {
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

                if(expectedFullType.size() != typedArray->typeTokens->size()) return false;
                for(size_t i = 0; i < expectedFullType.size(); i++) {
                    if(expectedFullType[i].type != (*typedArray->typeTokens)[i].type) return false;
                }
                
                index = tempIdx;
                return true;
            }

            std::vector<PortalgValue>& vec = *(typedArray->elements);
            
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

std::vector<Token> Interpreter::extractSubtype(const std::vector<Token>& fullType) {
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

void Interpreter::validateType(const std::vector<Token>& typeTokens, PortalgValue& value, Token errorToken) {
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

void Interpreter::enforceType(const std::vector<Token>& typeTokens, PortalgValue& value, Token errorToken) {
    if(typeTokens.empty()) return;

    if(std::holds_alternative<std::string>(value) && typeTokens.size() == 1) {
        std::string textValue = std::get<std::string>(value);
        try {
            if (typeTokens[0].type == TokenType::KW_INTEGER) value = std::stoll(textValue);
            else if (typeTokens[0].type == TokenType::KW_REAL) value = std::stod(textValue);
        } catch (...) {}
    }

    validateType(typeTokens, value, errorToken);
}

PortalgValue Interpreter::applyIncrementDecrement(const PortalgValue& currentValue, TokenType opType, Token errorToken) {
    if (!isReal(currentValue) && !isInteger(currentValue)) {
        throw RuntimeError(errorToken, "Operadores de incremento e decremento só podem ser aplicados a valores numéricos.");
    }
    if (isInteger(currentValue)) {
        long long val = std::get<long long>(currentValue);
        return (opType == TokenType::PLUS_PLUS) ? (val + 1) : (val - 1);
    } else {
        double val = std::get<double>(currentValue);
        return (opType == TokenType::PLUS_PLUS) ? (val + 1.0) : (val - 1.0);
    }
}

PortalgValue Interpreter::cloneValue(const PortalgValue& value) {
    if(std::holds_alternative<std::shared_ptr<TypedArray>>(value)) {
        auto originalArray = std::get<std::shared_ptr<TypedArray>>(value);
        std::vector<PortalgValue> clonedElements;
        for(const auto& el : *(originalArray->elements)) {
            clonedElements.push_back(cloneValue(el));
        }
        return std::make_shared<TypedArray>(TypedArray{originalArray->typeTokens, std::make_shared<std::vector<PortalgValue>>(clonedElements)});
    }
    return value;
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> innerEnvironment) {
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

Interpreter::Interpreter() {
    environment->define("escreva", std::shared_ptr<PortalgCallable>(std::make_shared<NativeEscreva>()), true, {{TokenType::KW_VOID, "", 0}});
    environment->define("escreval", std::shared_ptr<PortalgCallable>(std::make_shared<NativeEscreval>()), true, {{TokenType::KW_VOID, "", 0}});
    environment->define("leia", std::shared_ptr<PortalgCallable>(std::make_shared<NativeLeia>()), true, {{TokenType::KW_TEXT, "", 0}});
    environment->define("raiz", std::shared_ptr<PortalgCallable>(std::make_shared<NativeRaiz>()), true, {{TokenType::KW_REAL, "", 0}});
    environment->define("log", std::shared_ptr<PortalgCallable>(std::make_shared<NativeLog>()), true, {{TokenType::KW_REAL, "", 0}});
    environment->define("arredonda_cima", std::shared_ptr<PortalgCallable>(std::make_shared<NativeArredondaCima>()), true, {{TokenType::KW_REAL, "", 0}});
    environment->define("arredonda_baixo", std::shared_ptr<PortalgCallable>(std::make_shared<NativeArredondaBaixo>()), true, {{TokenType::KW_REAL, "", 0}});
    environment->define("aleatorio", std::shared_ptr<PortalgCallable>(std::make_shared<NativeAleatorio>()), true, {{TokenType::KW_REAL, "", 0}});
}

void Interpreter::resolve(Expr* expr, int depth) {
    locals[expr] = depth;
}

Interpreter::PortalgUserFunction::PortalgUserFunction(FunctionStmt* declaration, std::shared_ptr<Environment> closure) : declaration(declaration), closure(closure) {}

int Interpreter::PortalgUserFunction::arity() {
    return declaration->params.size();
}

PortalgValue Interpreter::PortalgUserFunction::call(Interpreter* interpreter, const std::vector<PortalgValue>& arguments) {
    std::shared_ptr<Environment> env = std::make_shared<Environment>(closure);
    for(size_t i = 0; i < declaration->params.size(); i++) {
        PortalgValue arg = arguments[i];
        if(std::holds_alternative<std::shared_ptr<PortalgRef>>(arg)) {
            auto ref = std::get<std::shared_ptr<PortalgRef>>(arg);
            PortalgValue realVal = ref->env->get(Token{TokenType::IDENTIFIER, ref->name, 0});
            interpreter->enforceType(declaration->params[i].type, realVal, declaration->params[i].name);
        } else {
            interpreter->enforceType(declaration->params[i].type, arg, declaration->params[i].name);
        }
        env->define(declaration->params[i].name.lexeme, arg, false, declaration->params[i].type);
    }
    interpreter->functionDepth++;

    try {
        interpreter->executeBlock(declaration->body, env);
    } catch(const ReturnException& e) {
        interpreter->functionDepth--;
        PortalgValue retVal = e.value;
        if(!(declaration->returnType[0].type == TokenType::KW_VOID)) {
            interpreter->enforceType(declaration->returnType, retVal, declaration->name);
        } else if(!std::holds_alternative<std::monostate>(retVal)) {
            throw RuntimeError(declaration->name, "A função é do tipo vazio, mas retornou um valor.");
        }
        return retVal;
    } catch(...) {
        interpreter->functionDepth--;
        throw;
    }

    interpreter->functionDepth--;

    if(!(declaration->returnType[0].type == TokenType::KW_VOID)) {
        throw RuntimeError(declaration->name, "A função exige um retorno, mas não retornou nenhum valor.");
    }

    return std::monostate{};
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    loopDepth = 0;
    switchDepth = 0;
    functionDepth = 0;
    for(const auto& statement : statements) {
        execute(statement.get());
    }
}

void Interpreter::execute(Stmt* stmt) {
    if(debugModeOn) {
        json stateJson = json::array();
        auto scopes = environment->getAllScopes();

        for(const auto& scope : scopes) {
            json scopeJson;
            for(const auto& pair : scope) {
                PortalgValue val = pair.second.value;

                if(std::holds_alternative<std::shared_ptr<PortalgCallable>>(val)) {
                    auto callable = std::get<std::shared_ptr<PortalgCallable>>(val);
                    if(!dynamic_cast<PortalgUserFunction*>(callable.get())) {
                        continue;
                    }
                }

                try {
                    scopeJson[pair.first] = stringify(val);
                } catch(...) {
                    scopeJson[pair.first] = "<indefinido>";
                }
            }
            stateJson.push_back(scopeJson);
        }

        int actualLine = stmt->line;
        std::string jsonString = stateJson.dump();

        #ifdef __EMSCRIPTEN__
        debugger_pause(jsonString.c_str(), actualLine);
        #endif
    }
    stmt->accept(this);
}

std::string Interpreter::stringify(const PortalgValue& operand) {
    return std::visit([&](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            return "nulo";
        } 
        else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } 
        else if constexpr (std::is_same_v<T, char>) {
            return std::string(1, arg);
        } 
        else if constexpr (std::is_same_v<T, long long>) {
            return std::to_string(arg);
        } 
        else if constexpr (std::is_same_v<T, double>) {
            std::string str = std::to_string(arg);
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') str.pop_back();
            return str;
        } 
        else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "verdadeiro" : "falso";
        } 
        else if constexpr (std::is_same_v<T, std::shared_ptr<TypedArray>>) {
            std::string result = "[";
            const auto& elements = *(arg->elements);
            for(size_t i = 0; i < elements.size(); i++) {
                result += stringify(elements[i]);
                if(i < elements.size() - 1) result += ", ";
            }
            result += "]";
            return result;
        } 
        else if constexpr (std::is_same_v<T, std::shared_ptr<PortalgCallable>>) {
            return "<função>";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<PortalgRef>>) {
            return "<referencia_memoria>";
        }
        else {
            return "Impossível converter para texto.";
        }
    }, operand);
}

PortalgValue Interpreter::visitLiteralExpr(LiteralExpr* expr) {
    return expr->value;
}

PortalgValue Interpreter::visitBinaryExpr(BinaryExpr* expr) {
    PortalgValue left = evaluate(expr->left.get());
    PortalgValue right = evaluate(expr->right.get());
    return calculateMathAndRelationals(left, expr->op, right);
}

PortalgValue Interpreter::visitUnaryExpr(UnaryExpr* expr) {
    PortalgValue right = evaluate(expr->right.get());
    switch(expr->op.type) {
        case TokenType::MINUS:
            if(isReal(right)) {
                return -std::get<double>(right);
            }

            if(isInteger(right)) {
                return -std::get<long long>(right);
            }

            throw RuntimeError(expr->op, "O operador unário '-' exige um valor numérico.");

        case TokenType::NOT:
            if(std::holds_alternative<bool>(right)) {
                return !std::get<bool>(right);
            }

            throw RuntimeError(expr->op, "O operador de negação exige um valor lógico.");

        default:
            break;
    }
    return std::monostate{};
}

PortalgValue Interpreter::visitPrefixPostfixExpr(PrefixPostfixExpr* expr) {
    if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr->target.get())) {
        auto it = locals.find(varExpr);
        bool isLocal = it != locals.end();
        int distance = isLocal ? it->second : 0;

        PortalgValue targetObj = isLocal ? environment->getAt(distance, varExpr->name.lexeme) : environment->get(varExpr->name);
        PortalgValue currentValue = targetObj;
        if(std::holds_alternative<std::shared_ptr<PortalgRef>>(targetObj)) {
            auto ref = std::get<std::shared_ptr<PortalgRef>>(targetObj);
            currentValue = ref->env->get({TokenType::IDENTIFIER, ref->name, 0});
        }

        PortalgValue newValue = applyIncrementDecrement(currentValue, expr->op.type, expr->op);

        if(std::holds_alternative<std::shared_ptr<PortalgRef>>(targetObj)) {
            auto ref = std::get<std::shared_ptr<PortalgRef>>(targetObj);
            ref->env->assign(Token{TokenType::IDENTIFIER, ref->name, 0}, newValue);
        } else {
            if(isLocal) environment->assignAt(distance, varExpr->name, newValue);
            else environment->assign(varExpr->name, newValue);
        }

        if(expr->isPrefix) {
            return newValue;
        } else {
            return currentValue;
        }
    } else if(IndexAccessExpr* indexExpr = dynamic_cast<IndexAccessExpr*>(expr->target.get())) {
        std::vector<long long> indexes;
        Expr* currentTarget = indexExpr->target.get();
        
        PortalgValue evaluatedIndex = evaluate(indexExpr->index.get());
        if(!isInteger(evaluatedIndex)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
        indexes.push_back(std::get<long long>(evaluatedIndex));

        while(IndexAccessExpr* idxAccess = dynamic_cast<IndexAccessExpr*>(currentTarget)) {
            PortalgValue idxVal = evaluate(idxAccess->index.get());
            if(!isInteger(idxVal)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
            indexes.insert(indexes.begin(), std::get<long long>(idxVal));
            currentTarget = idxAccess->target.get();
        }

        VariableExpr* varExpr = dynamic_cast<VariableExpr*>(currentTarget);
        if(!varExpr) throw RuntimeError(expr->op, "O alvo da atribuição de índice deve ser uma variável.");

        auto it = locals.find(varExpr);
        bool isLocal = it != locals.end();
        int distance = isLocal ? it->second : 0;
        PortalgValue targetObj = isLocal ? environment->getAt(distance, varExpr->name.lexeme) : environment->get(varExpr->name);
        PortalgValue* currentAny = &targetObj;
        
        for(size_t i = 0; i < indexes.size(); i++) {
            long long idx = indexes[i];
            
            if(std::holds_alternative<std::shared_ptr<TypedArray>>(*currentAny)) {
                auto arrayPtr = std::get<std::shared_ptr<TypedArray>>(*currentAny);
                if(idx < 0 || idx >= arrayPtr->elements->size()) throw RuntimeError(expr->op, "Índice do vetor fora dos limites.");
                
                if(i == indexes.size() - 1) {
                    PortalgValue& finalElement = (*arrayPtr->elements)[idx];
                    PortalgValue currentValue = finalElement;
                    PortalgValue newValue = applyIncrementDecrement(currentValue, expr->op.type, expr->op);

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

PortalgValue Interpreter::visitVariableExpr(VariableExpr* expr) {
    auto it = locals.find(expr);
    int distance = (it != locals.end()) ? it->second : 0;
    PortalgValue val = (it != locals.end()) ? environment->getAt(distance, expr->name.lexeme) : environment->get(expr->name);
    if(std::holds_alternative<std::shared_ptr<PortalgRef>>(val)) {
        auto ref = std::get<std::shared_ptr<PortalgRef>>(val);
        return ref->env->get(Token{TokenType::IDENTIFIER, ref->name, 0});
    }
    return val;
}

PortalgValue Interpreter::visitLogicalExpr(LogicalExpr* expr) {
    PortalgValue left = evaluate(expr->left.get());
    if(!std::holds_alternative<bool>(left)) {
        throw RuntimeError(expr->op, "Tentativa de usar operador lógico com valores não lógicos.");
    }

    bool leftVal = std::get<bool>(left);

    if(expr->op.type == TokenType::OR) {
        if(leftVal) return true;
    } else {
        if(!leftVal) return false;
    }

    PortalgValue right = evaluate(expr->right.get());
    if(!std::holds_alternative<bool>(right)) {
        throw RuntimeError(expr->op, "Tentativa de usar operador lógico com valores não lógicos.");
    }

    return std::get<bool>(right);
}

PortalgValue Interpreter::visitTernaryExpr(TernaryExpr* expr) {
    PortalgValue condition = evaluate(expr->condition.get());
    if(!std::holds_alternative<bool>(condition)) {
        throw RuntimeError(expr->query, "A condição do operador ternário deve ser um valor lógico.");
    }

    if(std::get<bool>(condition)) {
        return evaluate(expr->trueExpr.get());
    } else {
        return evaluate(expr->falseExpr.get());
    }
}

void Interpreter::visitExpressionStmt(ExpressionStmt* stmt) {
    evaluate(stmt->expression.get());
}

void Interpreter::visitVarDeclStmt(VarDeclStmt* stmt) {
    PortalgValue value = evaluate(stmt->initializer.get());
    enforceType(stmt->type, value, stmt->name);
    environment->define(stmt->name.lexeme, std::move(value), stmt->isConst, stmt->type);
}

PortalgValue Interpreter::visitAssignExpr(AssignExpr* expr) {
    PortalgValue value = evaluate(expr->value.get());
    auto it = locals.find(expr);
    bool isLocal = it != locals.end();
    int distance = isLocal ? it->second : 0;

    PortalgValue targetObj = isLocal ? environment->getAt(distance, expr->name.lexeme) : environment->get(expr->name);

    PortalgValue actualValueForMath = targetObj;
    if(std::holds_alternative<std::shared_ptr<PortalgRef>>(targetObj)) {
        auto ref = std::get<std::shared_ptr<PortalgRef>>(targetObj);
        actualValueForMath = ref->env->get(Token{TokenType::IDENTIFIER, ref->name, 0});
    }

    if(expr->op.type != TokenType::EQUAL) {
        value = calculateMathAndRelationals(actualValueForMath, expr->op, value);
    }

    std::vector<Token> varType = isLocal ? environment->getTypeAt(distance, expr->name.lexeme) : environment->getType(expr->name);
    enforceType(varType, value, expr->name);

    if(std::holds_alternative<std::shared_ptr<PortalgRef>>(targetObj)) {
        auto ref = std::get<std::shared_ptr<PortalgRef>>(targetObj);
        ref->env->assign(Token{TokenType::IDENTIFIER, ref->name, 0}, value);
    } else {
        if(isLocal) environment->assignAt(distance, expr->name, value);
        else environment->assign(expr->name, value);
    }

    return value;
}

PortalgValue Interpreter::visitCallExpr(CallExpr* expr) {
    PortalgValue callee = evaluate(expr->callee.get());

    if(!std::holds_alternative<std::shared_ptr<PortalgCallable>>(callee)) {
        throw RuntimeError(expr->openToken, "Só é permitido chamar funções e métodos.");
    }

    auto function = std::get<std::shared_ptr<PortalgCallable>>(callee);

    auto userFunc = std::dynamic_pointer_cast<PortalgUserFunction>(function);

    std::vector<PortalgValue> arguments;
    for(size_t i = 0; i < expr->arguments.size(); i++) {
        if(userFunc && userFunc->declaration->params[i].isReference) {
            VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr->arguments[i].get());
            if(!varExpr) {
                throw RuntimeError(expr->openToken, "Parâmetros por referência exigem variáveis puras (não é possível passar valores diretos ou posições de vetores).");
            }
            auto it = locals.find(varExpr);
            bool isLocal = it != locals.end();
            int distance = isLocal ? it->second : 0;
            std::shared_ptr<Environment> targetEnv = isLocal ? environment->ancestorShared(distance) : environment;
            arguments.emplace_back(std::make_shared<PortalgRef>(PortalgRef{targetEnv, varExpr->name.lexeme}));
        } else {
            arguments.emplace_back(evaluate(expr->arguments[i].get()));
        }
    }


    if(function->arity() != -1 && arguments.size() != function->arity()) {
        throw RuntimeError(expr->openToken, "A função esperava " + std::to_string(function->arity()) + " argumentos, mas recebeu " + std::to_string(arguments.size()) + ".");
    }

    return function->call(this, arguments);
}

PortalgValue Interpreter::visitMethodCallExpr(MethodCallExpr* expr) {
    PortalgValue target = evaluate(expr->object.get());
    
    std::vector<PortalgValue> arguments;
    for(const auto& argExpr : expr->arguments) {
        arguments.emplace_back(evaluate(argExpr.get()));
    }
    std::string methodName = expr->methodName.lexeme;

    if(std::holds_alternative<std::string>(target)) {
        std::string text = std::get<std::string>(target);

        if(methodName == "tamanho") {
            if(!arguments.empty()) {
                throw RuntimeError(expr->methodName, "O método 'tamanho' não espera argumentos.");
            }
            return (long long)text.length();
        }

        if(methodName == "maiusculo") {
            if(!arguments.empty()) {
                throw RuntimeError(expr->methodName, "O método 'maiusculo' não espera argumentos.");
            }
            std::string result = text;
            for(char& c : result) c = std::toupper(static_cast<unsigned char>(c));
            return result;
        }

        if(methodName == "minusculo") {
            if(!arguments.empty()) {
                throw RuntimeError(expr->methodName, "O método 'minusculo' não espera argumentos.");
            }
            std::string result = text;
            for(char& c : result) c = std::tolower(static_cast<unsigned char>(c));
            return result;
        }

        if(methodName == "subtexto") {
            if(arguments.size() != 2) {
                throw RuntimeError(expr->methodName, "O método 'subtexto' espera 2 argumentos: índice inicial e índice final.");
            }

            if(!isInteger(arguments[0]) || !isInteger(arguments[1])) {
                throw RuntimeError(expr->methodName, "Os argumentos do método 'subtexto' devem ser números inteiros.");
            }

            long long init = std::get<long long>(arguments[0]);
            long long finalIdx = std::get<long long>(arguments[1]);

            if(init < 0 || init > finalIdx || finalIdx >= (long long)text.length()) {
                throw RuntimeError(expr->methodName, "Os índices do método 'subtexto' estão fora dos limites.");
            }

            long long length = finalIdx - init + 1;
            return text.substr(init, length);
        }

        throw RuntimeError(expr->methodName, "O método '" + methodName + "' não existe para o tipo texto.");
    }

    if(std::holds_alternative<std::shared_ptr<TypedArray>>(target)) {
        auto typedArray = std::get<std::shared_ptr<TypedArray>>(target);

        if(methodName == "tamanho") {
            if(!arguments.empty()) {
                throw RuntimeError(expr->methodName, "O método 'tamanho' não espera argumentos.");
            }
            return (long long)typedArray->elements->size();
        }

        if(methodName == "copia") {
            if(!arguments.empty()) {
                throw RuntimeError(expr->methodName, "O método 'copia' não espera argumentos.");
            }
            return cloneValue(target);
        }

        if(methodName == "adicionar") {
            if(arguments.size() != 1) {
                throw RuntimeError(expr->methodName, "O método 'adicionar' espera 1 argumento: o elemento a ser adicionado.");
            }

            PortalgValue element = arguments[0];

            if(!typedArray->typeTokens->empty()) {
                std::vector<Token> expectedType = extractSubtype(*typedArray->typeTokens);
                enforceType(expectedType, element, expr->methodName);
            }

            typedArray->elements->push_back(std::move(element));
            return std::monostate{};
        }

        if(methodName == "remover") {
            if(arguments.size() != 1) {
                throw RuntimeError(expr->methodName, "O método 'remover' espera exatamente 1 argumento: o índice do elemento a ser removido.");
            }

            if(!isInteger(arguments[0])) {
                throw RuntimeError(expr->methodName, "O argumento do método 'remover' deve ser um número inteiro.");
            }

            long long index = std::get<long long>(arguments[0]);

            if(index < 0 || index >= (long long)typedArray->elements->size()) {
                throw RuntimeError(expr->methodName, "Índice de remoção fora dos limites do vetor.");
            }

            PortalgValue removedValue = (*typedArray->elements)[index]; 
            typedArray->elements->erase(typedArray->elements->begin() + index);
            return removedValue;
        }

        throw RuntimeError(expr->methodName, "O método '" + methodName + "' não existe para o tipo vetor.");
    }

    throw RuntimeError(expr->methodName, "Tipo de dado não possui métodos.");
}

PortalgValue Interpreter::visitIndexAccessExpr(IndexAccessExpr* expr) {
    PortalgValue target = evaluate(expr->target.get());
    PortalgValue indexValue = evaluate(expr->index.get());

    if(!isInteger(indexValue)) {
        throw RuntimeError(expr->bracketToken, "O índice deve ser um número inteiro positivo.");
    }

    long long index = std::get<long long>(indexValue);

    if(std::holds_alternative<std::string>(target)) {
        std::string text = std::get<std::string>(target);
        if(index < 0 || index >= text.length()) {
            throw RuntimeError(expr->bracketToken, "Índice de texto fora dos limites.");
        }
        return text[index];
    }

    if(std::holds_alternative<std::shared_ptr<TypedArray>>(target)) {
        auto typedArray = std::get<std::shared_ptr<TypedArray>>(target);
        if(index < 0 || index >= typedArray->elements->size()) {
            throw RuntimeError(expr->bracketToken, "Índice do vetor fora dos limites.");
        }
        return (*typedArray->elements)[index];
    }

    throw RuntimeError(expr->bracketToken, "Só é possível acessar índices de vetores ou textos.");
}

PortalgValue Interpreter::visitIndexAssignExpr(IndexAssignExpr* expr) {
    std::vector<long long> indexes;
    Expr* currentTarget = expr->target.get();
    
    PortalgValue evaluatedIndex = evaluate(expr->index.get());
    if(!isInteger(evaluatedIndex)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
    indexes.push_back(std::get<long long>(evaluatedIndex));

    while(IndexAccessExpr* idxAccess = dynamic_cast<IndexAccessExpr*>(currentTarget)) {
        PortalgValue idxVal = evaluate(idxAccess->index.get());
        if(!isInteger(idxVal)) throw RuntimeError(expr->op, "O índice deve ser um número inteiro positivo.");
        indexes.insert(indexes.begin(), std::get<long long>(idxVal));
        currentTarget = idxAccess->target.get();
    }

    VariableExpr* varExpr = dynamic_cast<VariableExpr*>(currentTarget);
    if(!varExpr) throw RuntimeError(expr->op, "O alvo da atribuição de índice deve ser uma variável.");

    auto it = locals.find(varExpr);
    bool isLocal = it != locals.end();
    int distance = isLocal ? it->second : 0;

    PortalgValue targetObj = isLocal ? environment->getAt(distance, varExpr->name.lexeme) : environment->get(varExpr->name);
    PortalgValue newValue = evaluate(expr->value.get());
    PortalgValue* currentAny = &targetObj;
    
    for(size_t i = 0; i < indexes.size(); i++) {
        long long idx = indexes[i];
        
        if(std::holds_alternative<std::shared_ptr<TypedArray>>(*currentAny)) {
            auto arrayPtr = std::get<std::shared_ptr<TypedArray>>(*currentAny);
            if(idx < 0 || idx >= arrayPtr->elements->size()) throw RuntimeError(expr->op, "Índice do vetor fora dos limites.");
            
            if(i == indexes.size() - 1) {
                PortalgValue& finalElement = (*arrayPtr->elements)[idx];
                
                if(expr->op.type != TokenType::EQUAL) {
                    newValue = calculateMathAndRelationals(finalElement, expr->op, newValue);
                }

                std::vector<Token> currentExpectedType = isLocal ? environment->getTypeAt(distance, varExpr->name.lexeme) : environment->getType(varExpr->name);
                for(size_t d = 0; d <= i; d++) {
                    currentExpectedType = extractSubtype(currentExpectedType);
                }
                enforceType(currentExpectedType, newValue, expr->op);
                
                finalElement = newValue;
                return newValue;
            } else {
                currentAny = &((*arrayPtr->elements)[idx]);
            }
        } else if(std::holds_alternative<std::string>(*currentAny)) {
            if(i != indexes.size() - 1) throw RuntimeError(expr->op, "Não é possível acessar índices dentro de um texto.");
            
            std::string* strPtr = &std::get<std::string>(*currentAny);
            if(idx < 0 || idx >= strPtr->length()) throw RuntimeError(expr->op, "Índice de texto fora dos limites.");
            
            if(expr->op.type != TokenType::EQUAL) {
                PortalgValue currentChar = std::string(1, (*strPtr)[idx]);
                newValue = calculateMathAndRelationals(currentChar, expr->op, newValue);
            }

            if(std::holds_alternative<char>(newValue)) {
                (*strPtr)[idx] = std::get<char>(newValue);
            } else if(std::holds_alternative<std::string>(newValue) && std::get<std::string>(newValue).length() == 1) {
                (*strPtr)[idx] = std::get<std::string>(newValue)[0];
            } else {
                throw RuntimeError(expr->op, "Apenas caracteres podem ser atribuídos a índices de texto.");
            }
            
            if(isLocal) {
                environment->assignAt(distance, varExpr->name, targetObj);
            } else {
                environment->assign(varExpr->name, targetObj);
            }
            return newValue;
        } else {
            throw RuntimeError(expr->op, "Tentativa de acessar índice de um tipo não indexável.");
        }
    }
    return newValue;
}

PortalgValue Interpreter::visitArrayLiteralExpr(ArrayLiteralExpr* expr) {
    std::vector<PortalgValue> elements;
    for(const auto& elementExpr : expr->elements) {
        elements.emplace_back(evaluate(elementExpr.get()));
    }
    return std::make_shared<TypedArray>(TypedArray{std::make_shared<std::vector<Token>>(), std::make_shared<std::vector<PortalgValue>>(elements)});
}

PortalgValue Interpreter::visitInstantiateExpr(InstantiateExpr* expr) {
    PortalgValue sizeValue = evaluate(expr->size.get());
    if(!isInteger(sizeValue)) {
        throw RuntimeError(expr->type[0], "O tamanho do vetor deve ser um número inteiro positivo.");
    }

    long long size = std::get<long long>(sizeValue);
    if(size < 0) {
        throw RuntimeError(expr->type[0], "O tamanho do vetor deve ser positivo.");
    }

    PortalgValue initialValue = evaluate(expr->initialValue.get());

    std::vector<Token> expectedSubType = extractSubtype(expr->type);
    validateType(expectedSubType, initialValue, expr->type[0]);

    std::vector<PortalgValue> elements;
    for(long long i = 0; i < size; i++) {
        elements.push_back(cloneValue(initialValue));
    }
    return std::make_shared<TypedArray>(TypedArray{std::make_shared<std::vector<Token>>(expr->type), std::make_shared<std::vector<PortalgValue>>(elements)});
}

void Interpreter::visitBlockStmt(BlockStmt* stmt) {
    executeBlock(stmt->statements, std::make_shared<Environment>(this->environment));
}

void Interpreter::visitIfStmt(IfStmt* stmt) {
    PortalgValue conditionResult = evaluate(stmt->condition.get());
    if(!std::holds_alternative<bool>(conditionResult)) {
        throw RuntimeError(stmt->ifToken, "A condição do 'se' deve retornar um valor lógico.");
    }

    if(std::get<bool>(conditionResult)) {
        execute(stmt->thenBranch.get());
    } else if(stmt->elseBranch != nullptr) {
        execute(stmt->elseBranch.get());
    }
}

void Interpreter::visitWhileStmt(WhileStmt* stmt) {
    loopDepth++;
    try {
        while(true) {
            PortalgValue conditionResult = evaluate(stmt->condition.get());

            if(!std::holds_alternative<bool>(conditionResult)) {
                throw RuntimeError(stmt->keyword, "A condição do laço de repetição deve ser um valor lógico.");
            }

            if(!std::get<bool>(conditionResult)) {
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

void Interpreter::visitForStmt(ForStmt* stmt) {
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
                PortalgValue conditionResult = evaluate(stmt->condition.get());
                
                if(!std::holds_alternative<bool>(conditionResult)) {
                    throw RuntimeError(stmt->keyword, "A condição do laço de repetição deve ser um valor lógico.");
                }

                if(!std::get<bool>(conditionResult)) {
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

void Interpreter::visitBreakStmt(BreakStmt* stmt) {
    if(loopDepth == 0 && switchDepth == 0) {
        throw RuntimeError(stmt->keyword, "O comando 'parar' só pode ser utilizado dentro de laços de repetição ou bloco 'escolha'.");
    }
    throw BreakException();
}

void Interpreter::visitContinueStmt(ContinueStmt* stmt) {
    if(loopDepth == 0) {
        throw RuntimeError(stmt->keyword, "O comando 'continuar' só pode ser utilizado dentro de laços de repetição.");
    }
    throw ContinueException();
}

void Interpreter::visitReturnStmt(ReturnStmt* stmt) {
    if(functionDepth == 0) {
        throw RuntimeError(stmt->keyword, "O comando 'retornar' só pode ser utilizado dentro de funções.");
    }

    PortalgValue value = std::monostate{};

    if(stmt->value != nullptr) {
        value = evaluate(stmt->value.get());
    }

    throw ReturnException(value);
}

void Interpreter::visitFunctionStmt(FunctionStmt* stmt) {
    std::shared_ptr<PortalgCallable> function = std::make_shared<PortalgUserFunction>(stmt, this->environment);
    environment->define(stmt->name.lexeme, function, true, stmt->returnType);
}

void Interpreter::visitSwitchStmt(SwitchStmt* stmt) {
    PortalgValue switchValue = evaluate(stmt->target.get());
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
                    PortalgValue caseValue = evaluate(switchCase.matchExpr.get());
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