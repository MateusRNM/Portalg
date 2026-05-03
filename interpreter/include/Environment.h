#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include <stdexcept>
#include <vector>
#include "Token.h"

class RuntimeError : public std::runtime_error {
    public:
        Token token;
        RuntimeError(Token token, std::string message) : std::runtime_error(message), token(token) {}
};

struct VariableData {
    std::any value;
    bool isConst;
    std::vector<Token> type;
};

class Environment : public std::enable_shared_from_this<Environment> {
    private:
        std::unordered_map<std::string, VariableData> values;
        std::shared_ptr<Environment> enclosing;
    public:
        Environment() : enclosing(nullptr) {}
        Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

        void define(const std::string& name, std::any value, bool isConst, std::vector<Token> type) {
            values[name] = {std::move(value), isConst, type};
        }

        std::any get(Token name) {
            auto it = values.find(name.lexeme);
            if(it != values.end()) {
                return it->second.value;
            }

            if(enclosing != nullptr) {
                return enclosing->get(name);
            }

            throw RuntimeError(name, "Variável indefinida: '" + name.lexeme + "'.");
        }

        std::vector<Token> getType(Token name) {
            auto it = values.find(name.lexeme);
            if(it != values.end()) {
                return it->second.type;
            }
            
            if(enclosing != nullptr) { 
                return enclosing->getType(name);
            }
            
            throw RuntimeError(name, "Variável indefinida: '" + name.lexeme + "'.");
        }

        void assign(Token name, std::any value) {
            auto it = values.find(name.lexeme);
            if(it != values.end()) {
                if(it->second.isConst) {
                    throw RuntimeError(name, "Tentativa de reatribuição à constante '" + name.lexeme + "'.");
                }
                it->second.value = std::move(value);
                return;
            }

            if(enclosing != nullptr) {
                enclosing->assign(name, std::move(value));
                return;
            }

            throw RuntimeError(name, "Tentativa de atribuição a uma variável indefinida: '" + name.lexeme + "'.");
        }

        std::unordered_map<std::string, VariableData> getLocals() const {
            return values;
        }

        std::shared_ptr<Environment> getEnclosing() const {
            return enclosing;
        }
};