#pragma once
#include <vector>
#include <string>
#include <memory>
#include <initializer_list>
#include <stdexcept>
#include "Token.h"
#include "Stmt.h"
#include "ErrorHandler.h"

class ParseError : public std::runtime_error {
    public:
        ParseError() : std::runtime_error("Parse Error") {}
};

class Parser {
    private:
        std::vector<Token> tokens;
        int current = 0;
        ErrorHandler& errorHandler;
        bool isAtEnd();
        Token previous();
        Token peek();
        Token advance();
        bool check(TokenType type);
        bool match(std::initializer_list<TokenType> types);
        ParseError error(Token token, std::string message);
        Token consume(TokenType type, std::string message);
        void synchronize();
        std::unique_ptr<Expr> expression();
        std::unique_ptr<Expr> assignment();
        std::unique_ptr<Expr> ternary();
        std::unique_ptr<Expr> logic_or();
        std::unique_ptr<Expr> logic_and();
        std::unique_ptr<Expr> equality();
        std::unique_ptr<Expr> comparison();
        std::unique_ptr<Expr> term();
        std::unique_ptr<Expr> factor();
        std::unique_ptr<Expr> exponent();
        std::unique_ptr<Expr> unary();
        std::unique_ptr<Expr> postfix();
        std::unique_ptr<Expr> call();
        std::unique_ptr<Expr> primary();
        std::vector<std::unique_ptr<Stmt>> declaration();
        std::vector<std::unique_ptr<Stmt>> constDecl();
        std::vector<std::unique_ptr<Stmt>> varDecl(std::vector<Token> declType, Token name);
        std::unique_ptr<Stmt> funcDecl(std::vector<Token> declType, Token name);
        std::vector<FunctionParam> parameters();
        std::vector<Token> type();
        std::unique_ptr<Stmt> statement();
        std::unique_ptr<Stmt> exprStmt();
        std::unique_ptr<Stmt> ifStmt();
        std::unique_ptr<Stmt> whileStmt();
        std::unique_ptr<Stmt> forStmt();
        std::unique_ptr<Stmt> switchStmt();
        CaseClause caseClause();
        std::vector<std::unique_ptr<Stmt>> defaultClause();
        std::unique_ptr<Stmt> block();
        std::unique_ptr<Stmt> returnStmt();
        std::unique_ptr<Stmt> breakStmt();
        std::unique_ptr<Stmt> continueStmt();
    public:
        Parser(const std::vector<Token>& tokens, ErrorHandler& errorHandler);
        std::vector<std::unique_ptr<Stmt>> parse();
};