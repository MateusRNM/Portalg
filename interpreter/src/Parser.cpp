#include "Parser.h"
#include "Expr.h"

Parser::Parser(const std::vector<Token>& tokens, ErrorHandler& errorHandler) : tokens(tokens), errorHandler(errorHandler) {
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current-1];
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) {
    if(isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if(!isAtEnd()) current++;
    return previous();
}

ParseError Parser::error(Token token, std::string message) {
    errorHandler.error(token.line, token.column, message);
    return ParseError();
}

Token Parser::consume(TokenType type, std::string message) {
    if(check(type)) return advance();
    throw error(peek(), message);
}

void Parser::synchronize() {
    advance();

    while(!isAtEnd()) {
        if(previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::BREAK:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::KW_CHAR:
            case TokenType::KW_CONST:
            case TokenType::KW_INTEGER:
            case TokenType::KW_LOGIC:
            case TokenType::KW_REAL:
            case TokenType::KW_VECTOR:
            case TokenType::KW_VOID:
            case TokenType::KW_TEXT:
            case TokenType::SWITCH:
            case TokenType::WHILE:
            case TokenType::NEWLINE:
            case TokenType::RETURN:
                return;
        }

        advance();
    }
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for(TokenType type : types) {
        if(check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = ternary();
    if(match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::SLASH_EQUAL, TokenType::STAR_EQUAL, TokenType::POTENCY_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> value = assignment();
        if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(varExpr->name, op, std::move(value));
        } else if(IndexAccessExpr* indexAccessExpr = dynamic_cast<IndexAccessExpr*>(expr.get())) {
            return std::make_unique<IndexAssignExpr>(indexAccessExpr->name, std::move(indexAccessExpr->index), op, std::move(value));
        }

        throw error(op, "Atribuição inválida.");
    }
    return expr;
}

std::unique_ptr<Stmt> Parser::statement() {
    if(match({TokenType::IF})) return ifStmt();
    if(match({TokenType::WHILE})) return whileStmt();
    if(match({TokenType::FOR})) return forStmt();
    if(match({TokenType::RETURN})) return returnStmt();
    if(match({TokenType::BREAK})) return breakStmt();
    if(match({TokenType::CONTINUE})) return continueStmt();
    if(match({TokenType::SWITCH})) return switchStmt();
    if(match({TokenType::LEFT_BRACE})) return block();
    return exprStmt();
}

std::vector<std::unique_ptr<Stmt>> Parser::declaration() {
    std::vector<std::unique_ptr<Stmt>> declarations;
    if(check(TokenType::KW_CONST)) {
        for(auto& st : constDecl()) {
            declarations.emplace_back(std::move(st));
        }
    } else if(check(TokenType::KW_CHAR) || check(TokenType::KW_INTEGER) || check(TokenType::KW_LOGIC) || check(TokenType::KW_REAL) || check(TokenType::KW_TEXT) || check(TokenType::KW_VECTOR) || check(TokenType::KW_VOID)) {
        std::vector<Token> declType = type();
        Token name = consume(TokenType::IDENTIFIER, "Nome da variável ou função esperado.");
        if(check(TokenType::LEFT_PAREN)) {
            declarations.emplace_back(funcDecl(declType, name));
        } else {
            if(declType[0].type == TokenType::KW_VOID) {
                throw error(declType[0], "Não é possível inicializar uma variável do tipo vazio.");
            }
            for(auto& st : varDecl(declType, name)) {
                declarations.emplace_back(std::move(st));
            }
        }
    } else {
        declarations.emplace_back(statement());
    }
    return declarations;
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while(!isAtEnd()) {
        if(match({TokenType::NEWLINE})) continue;

        try {
            for(auto& st : declaration()) {
                statements.emplace_back(std::move(st));
            } 
        } catch(ParseError& error) {
            synchronize();
        }
    }
    return statements;
}