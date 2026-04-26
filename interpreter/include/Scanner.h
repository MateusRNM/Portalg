#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Token.h"
#include "ErrorHandler.h"

class Scanner {
    private:
        std::string source;
        std::vector<Token> tokens;
        ErrorHandler& errorHandler;
        int start = 0;
        int startLineGlobal = 1;
        int startColumnGlobal = 1;
        int current = 0;
        int line = 1;
        int column = 0;
        static const std::unordered_map<std::string, TokenType> keywords;

        bool isAtEnd();
        void scanToken();
        void addToken(TokenType type, std::string text = "");
        char advance();
        bool match(char expected);
        char peek();
        void string();
        void character();
        bool isDigit(char c);
        void number();
        char peekNext();
        void identifier();
        bool isAlpha(char c);
        bool isAlphaNumeric(char c);

    public:
        Scanner(const std::string& source, ErrorHandler& errorHandler);
        std::vector<Token> scanTokens();
};