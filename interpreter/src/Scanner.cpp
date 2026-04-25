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
    column++;
    return source[current++];
}

bool Scanner::match(char expected) {
    if(isAtEnd()) return false;
    if(source[current] != expected) return false;
    current++;
    return true;
}

char Scanner::peek() {
    if(isAtEnd()) return '\0';
    return source[current];
}

char Scanner::peekNext() {
    if(isAtEnd()) return '\0';
    return source[current+1];
}

void Scanner::addToken(TokenType type) {
    std::string text = source.substr(start, current-start);
    int startColumn = column - text.length();
    if(startColumn < 1) startColumn = 1;
    tokens.emplace_back(Token{type, text, line, startColumn});
}

void Scanner::scanToken() {
    char c = advance();
    switch(c) {
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            column = 0;
            break;
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case ':': addToken(TokenType::COLON); break;
        case '?': addToken(TokenType::QUERY); break;
        case '&': addToken(TokenType::AMPERSAND); break;
        case '+':
            if(match('+')) {
                addToken(TokenType::PLUS_PLUS);
            } else if(match('=')) {
                addToken(TokenType::PLUS_EQUAL);
            } else {
                addToken(TokenType::PLUS);
            }
            break;
        case '-':
            if(match('-')) {
                addToken(TokenType::MINUS_MINUS);
            } else if(match('=')) {
                addToken(TokenType::MINUS_EQUAL);
            } else {
                addToken(TokenType::MINUS);
            }
            break;
        case '*': 
            if(match('*')) {
                addToken(match('=') ? TokenType::POTENCY_EQUAL : TokenType::POTENCY); 
            } else {
                addToken(match('=') ? TokenType::STAR_EQUAL : TokenType::STAR); 
            }
            break;
        case '/':
            if(match('/')) {
                while(peek() != '\n' && !isAtEnd()) advance();
                line++;
                column = 0;
            } else if(match('*')) {
                while(!isAtEnd()) {
                    if(peek() == '\n') {
                        line++;
                        column = 0;
                    }

                    if(peek() == '*' && peekNext() == '/') {
                        advance();
                        advance();
                        break;
                    }

                    advance();
                }
            } else {
                addToken(match('=') ? TokenType::SLASH_EQUAL : TokenType::SLASH); 
            }
            break;
        case '%': addToken(match('=') ? TokenType::MOD_EQUAL : TokenType::MOD); break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '!': addToken(match('=') ? TokenType::NOT_EQUAL : TokenType::NOT); break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        default:
            errorHandler.error(line, column, "Caractere inesperado: " + c);
            break;
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