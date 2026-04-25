#include <iostream>
#include "Scanner.h"
#include "ErrorHandler.h"

int main() {
    std::string code = "// =\n [] /* {{{{{}}}}} = + +=*/ ==";
    ErrorHandler errorHandler = ErrorHandler();
    Scanner scanner(code, errorHandler);
    
    std::vector<Token> tokens = scanner.scanTokens();

    for(Token t : tokens) {
        std::cout << t.lexeme << '\n';
    }
    
    return 0;
}