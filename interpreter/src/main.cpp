#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Scanner.h"
#include "ErrorHandler.h"
#include "Token.h"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_INTEGER: return "KW_INTEGER";
        case TokenType::KW_REAL: return "KW_REAL";
        case TokenType::KW_TEXT: return "KW_TEXT";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::LITERAL_INTEGER: return "LITERAL_INTEGER";
        case TokenType::LITERAL_REAL: return "LITERAL_REAL";
        case TokenType::LITERAL_TEXT: return "LITERAL_TEXT";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::PLUS: return "PLUS";
        case TokenType::EOF_TOKEN: return "EOF_TOKEN";
        default: return "TOKEN_ID(" + std::to_string(static_cast<int>(type)) + ")";
    }
}

void run(const std::string& source) {
    ErrorHandler errorHandler;
    Scanner scanner(source, errorHandler);
    
    std::vector<Token> tokens = scanner.scanTokens();

    std::cout << "--- TOKENS ---\n";
    for (const Token& token : tokens) {
        std::cout << "[Linha " << token.line << " | Col " << token.column << "] " << tokenTypeToString(token.type) << " -> Lexema: '" << token.lexeme << "'\n";
    }
}

void runFile(const char* path) {
    std::ifstream file(path);
    
    if (!file.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo '" << path << "'\n";
        exit(74);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    run(buffer.str());
}

void runPrompt() {
    std::string line;
    std::cout << "Portalg REPL (Digite 'sair' para encerrar)\n";
    
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line) || line == "sair") break;
        run(line);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }
    return 0;
}