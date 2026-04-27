#include <iostream>
#include "ErrorHandler.h"

void ErrorHandler::report() {
    std::cout << errors.size() << " erros encontrados:";
    for(Error err : errors) {
        std::cout << "\n[ERRO: Linha " << err.line << " | Col " << err.column << "] " << err.message;
    }
}

bool ErrorHandler::haveErrors() {
    return !errors.empty();
}

void ErrorHandler::error(int line, int column, std::string message) {
    errors.emplace_back(Error{line, column, message});
}