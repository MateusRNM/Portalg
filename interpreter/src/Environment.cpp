#include "Environment.h"

Environment* Environment::ancestor(int distance) {
    Environment* environment = this;
    for(int i = 0; i < distance; i++) {
        environment = environment->enclosing.get();
    }
    return environment;
}

std::shared_ptr<Environment> Environment::ancestorShared(int distance) {
    std::shared_ptr<Environment> environment = shared_from_this();
    for(int i = 0; i < distance; i++) {
        environment = environment->enclosing;
    }
    return environment;
}

Environment::Environment() : enclosing(nullptr) {}

Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

void Environment::define(const std::string& name, std::any value, bool isConst, std::vector<Token> type) {
    values[name] = {std::move(value), isConst, type};
}

std::any Environment::getAt(int distance, const std::string& name) {
    return ancestor(distance)->values.at(name).value;
}

void Environment::assignAt(int distance, Token name, std::any value) {
    VariableData& varData = ancestor(distance)->values[name.lexeme];
    if(varData.isConst) {
        throw RuntimeError(name, "Tentativa de reatribuição à constante '" + name.lexeme + "'.");
    }
    varData.value = std::move(value);
}

std::vector<Token> Environment::getTypeAt(int distance, const std::string& name) {
    return ancestor(distance)->values.at(name).type;
}

std::any Environment::get(Token name) {
    auto it = values.find(name.lexeme);
    if(it != values.end()) {
        return it->second.value;
    }

    if(enclosing != nullptr) {
        return enclosing->get(name);
    }

    throw RuntimeError(name, "Variável indefinida: '" + name.lexeme + "'.");
}

std::vector<Token> Environment::getType(Token name) {
    auto it = values.find(name.lexeme);
    if(it != values.end()) {
        return it->second.type;
    }
    
    if(enclosing != nullptr) { 
        return enclosing->getType(name);
    }
    
    throw RuntimeError(name, "Variável indefinida: '" + name.lexeme + "'.");
}

void Environment::assign(Token name, std::any value) {
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

std::unordered_map<std::string, VariableData> Environment::getLocals() const {
    return values;
}

std::vector<std::unordered_map<std::string, VariableData>> Environment::getAllScopes() const {
    std::vector<std::unordered_map<std::string, VariableData>> scopes;
    const Environment* current = this;

    while(current != nullptr) {
        scopes.push_back(current->getLocals());
        current = current->enclosing.get();
    }
    
    return scopes;
}