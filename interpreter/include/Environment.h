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

class BreakException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Flux control: Break";
        }
};

class ContinueException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Flux control: Continue";
        }
};

class ReturnException : public std::exception {
    public:
        std::any value;
        ReturnException(std::any value) : value(std::move(value)) {}
        
        const char* what() const noexcept override {
            return "Flux control: Return";
        }
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

        Environment* ancestor(int distance);

    public:
        Environment();
        Environment(std::shared_ptr<Environment> enclosing);

        void define(const std::string& name, std::any value, bool isConst, std::vector<Token> type);
        std::any getAt(int distance, const std::string& name);
        void assignAt(int distance, Token name, std::any value);
        std::vector<Token> getTypeAt(int distance, const std::string& name);
        std::any get(Token name);
        std::vector<Token> getType(Token name);
        void assign(Token name, std::any value);
        std::unordered_map<std::string, VariableData> getLocals() const;
        std::vector<std::unordered_map<std::string, VariableData>> getAllScopes() const;
};