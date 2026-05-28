#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Scanner.h"
#include "Stmt.h"
#include "Parser.h"
#include "ErrorHandler.h"
#include "Token.h"
#include "Resolver.h"
#include "Interpreter.h"

void run(const std::string& source) {
    ErrorHandler errorHandler;
    Scanner scanner(source, errorHandler);
    
    std::vector<Token> tokens = scanner.scanTokens();

    if(errorHandler.haveErrors()) {
        errorHandler.report();
        return;
    }

    Parser parser(tokens, errorHandler);

    std::vector<std::unique_ptr<Stmt>> ast = parser.parse();

    if(errorHandler.haveErrors()) {
        errorHandler.report();
        return;
    }

    Interpreter interpreter;
    Resolver resolver(&interpreter);

    try {
        resolver.resolve(ast);
        interpreter.interpret(ast);
    } catch(const RuntimeError& error) {
        std::cerr << "Erro (Linha " << error.token.line << "): " << error.what() << "\n";
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
    std::cout << "Portalg REPL (Digite 'sair' para encerrar)";
    
    for (;;) {
        std::cout << "\n> ";
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