#include "Scanner.h"

const std::unordered_map<std::string, TokenType> keywords = {
    { "e", TokenType::AND },
    { "ou", TokenType::OR },
    { "inteiro", TokenType::KW_INTEGER },
    { "real", TokenType::KW_REAL },
    { "texto", TokenType::KW_TEXT },
    { "caractere", TokenType::KW_CHAR },
    { "logico", TokenType::KW_LOGIC },
    { "verdadeiro", TokenType::LITERAL_TRUE },
    { "falso", TokenType::LITERAL_FALSE },
    { "vetor", TokenType::KW_VECTOR },
    { "vazio", TokenType::KW_VOID },
    { "constante", TokenType::KW_CONST },
    { "se", TokenType::IF },
    { "senao", TokenType::ELSE },
    { "escolha", TokenType::SWITCH },
    { "caso", TokenType::CASE },
    { "outrocaso", TokenType::DEFAULT_CASE },
    { "enquanto", TokenType::WHILE },
    { "para", TokenType::FOR },
    { "parar", TokenType::BREAK },
    { "continuar", TokenType::CONTINUE },
    { "retornar", TokenType::RETURN }
};

Scanner::Scanner(const std::string& source, ErrorHandler& errorHandler) : source(source), errorHandler(errorHandler) {
}

bool Scanner::isAtEnd() {
    return current >= source.size();
}

char Scanner::advance() {
    return source[current++];
}

void Scanner::addToken(TokenType type) {
    std::string text = source.substr(start, current-start);
    tokens.emplace_back(Token{type, text, line, column});
}

void Scanner::scanToken() {
    char c = advance();
    column = current;
    switch(c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
    }
}

std::vector<Token> Scanner::scanTokens() {
    while(!isAtEnd()) {
        start = current;
        scanToken();
    }

    tokens.emplace_back(Token{TokenType::EOF_TOKEN, "", line, column});
    return tokens;
}