#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include <stdexcept>
#include "Token.h"

class RuntimeError : public std::runtime_error {
    public:
        Token token;
        RuntimeError(Token token, std::string message) : std::runtime_error(message), token(token) {}
};

class Environment : public std::enable_shared_from_this<Environment> {
    private:
        std::unordered_map<std::string, std::any> values;
        std::shared_ptr<Environment> enclosing;
    public:
        Environment() : enclosing(nullptr) {}
        Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

        void define(const std::string& name, std::any value) {
            values[name] = value;
        }

        std::any get(Token name) {
            auto it = values.find(name.lexeme);
            if(it != values.end()) {
                return it->second;
            }

            if(enclosing != nullptr) {
                return enclosing->get(name);
            }

            throw RuntimeError(name, "Variável indefinida: '" + name.lexeme + "'.");
        }

        void assign(Token name, std::any value) {
            auto it = values.find(name.lexeme);
            if(it != values.end()) {
                it->second = std::move(value);
                return;
            }

            if(enclosing != nullptr) {
                enclosing->assign(name, std::move(value));
                return;
            }

            throw RuntimeError(name, "Tentativa de atribuição a uma variável indefinida: '" + name.lexeme + "'.");
        }

        std::unordered_map<std::string, std::any> getLocals() const {
            return values;
        }

        std::shared_ptr<Environment> getEnclosing() const {
            return enclosing;
        }
};