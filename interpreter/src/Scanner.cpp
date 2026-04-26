#include "Scanner.h"

const std::unordered_map<std::string, TokenType> Scanner::keywords = {
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
    return current >= source.length();
}

char Scanner::advance() {
    column++;
    return source[current++];
}

bool Scanner::match(char expected) {
    if(isAtEnd()) return false;
    if(source[current] != expected) return false;
    advance();
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

bool Scanner::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

void Scanner::identifier() {
    while(isAlphaNumeric(peek())) advance();
    std::string text = source.substr(start, current-start);
    auto it = keywords.find(text);
    if(it != keywords.end()) {
        addToken(it->second);
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}

void Scanner::character() {
    std::string value = "";
    bool localError = false;
    while(peek() != '\'' && !isAtEnd()) {

        if(peek() == '\n') {
            line++;
            column = 0;
            if(!localError) {
                errorHandler.error(startLineGlobal, startColumnGlobal, "Quebra de linha não permitida dentro de um caractere.");
                localError = true;
            }
        }

        if(peek() == '\\') {
            advance();
            
            if(peek() == 'n') {
                value += '\n';
            } else {
                value += peek();
            }

            if(!isAtEnd()) {
                advance();
            }
        } else {
            value += peek();
            advance();
        }
    }

    if(isAtEnd()) {
        errorHandler.error(startLineGlobal, startColumnGlobal, "Caractere indeterminado.");
        return;
    }

    advance();

    if(value.length() > 1) {
        if(!localError) {
            errorHandler.error(startLineGlobal, startColumnGlobal, "Um caractere deve conter exatamente uma letra/símbolo.");
        }
        return;
    }

    if(localError) {
        return;
    }

    addToken(TokenType::LITERAL_CHAR, value);
}

void Scanner::string() {
    std::string value = "";
    while(peek() != '"' && !isAtEnd()) {
        if(peek() == '\n') {
            line++;
            column = 0;
        }

        if(peek() == '\\') {
            advance();

            if(peek() == 'n') {
                value += '\n';
            } else {
                value += peek();
            }

            if(!isAtEnd()) {
                advance();
            }
        } else {
            value += peek();
            advance();
        }
    }

    if(isAtEnd()) {
        errorHandler.error(startLineGlobal, startColumnGlobal, "Texto indeterminado.");
        return;
    }
    
    advance();

    addToken(TokenType::LITERAL_TEXT, value);
}

void Scanner::number() {
    while(isDigit(peek())) advance();

    if(peek() == '.' && isDigit(peekNext())) {
        advance();
        while(isDigit(peek())) advance();
        addToken(TokenType::LITERAL_REAL);
    } else {
        addToken(TokenType::LITERAL_INTEGER);
    }
}

void Scanner::addToken(TokenType type, std::string text) {
    if(text == "" && type != TokenType::LITERAL_CHAR && type != TokenType::LITERAL_TEXT) text = source.substr(start, current-start);
    tokens.emplace_back(Token{type, text, startLineGlobal, startColumnGlobal});
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
        case ';': addToken(TokenType::SEMICOLON); break;
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
            } else if(match('*')) {
                while(!isAtEnd()) {
                    if(peek() == '*' && peekNext() == '/') {
                        advance();
                        advance();
                        break;
                    }

                    if(advance() == '\n') {
                        line++;
                        column = 0;
                    }
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
        case '"': string(); break;
        case '\'': character(); break;
        default:
            if(isDigit(c)) {
                number();
            } else if(isAlpha(c)) {
                identifier();
            } else {
                std::string error_message = "Caractere inesperado: ";
                error_message += c;
                errorHandler.error(line, column, error_message);
            }
            break;
    }
}

std::vector<Token> Scanner::scanTokens() {
    while(!isAtEnd()) {
        start = current;
        startLineGlobal = line;
        startColumnGlobal = column == 0 ? 1 : column;
        scanToken();
    }

    tokens.emplace_back(Token{TokenType::EOF_TOKEN, "", line, column});
    return tokens;
}