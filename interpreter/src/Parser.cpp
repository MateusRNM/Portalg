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

std::unique_ptr<Expr> Parser::primary() {
    if(match({TokenType::LITERAL_FALSE, TokenType::LITERAL_TRUE})) {
        return std::make_unique<LiteralExpr>(previous().type == TokenType::LITERAL_FALSE ? false : true);
    } else if(match({TokenType::LITERAL_TEXT, TokenType::LITERAL_CHAR})) {
        Token literal = previous();
        if(literal.type == TokenType::LITERAL_TEXT) return std::make_unique<LiteralExpr>(literal.lexeme);
        else return std::make_unique<LiteralExpr>(literal.lexeme[0]);
    } else if(match({TokenType::LITERAL_INTEGER, TokenType::LITERAL_REAL})) {
        Token literal = previous();
        if(literal.type == TokenType::LITERAL_INTEGER) return std::make_unique<LiteralExpr>(std::stoll(literal.lexeme));
        else return std::make_unique<LiteralExpr>(std::stod(literal.lexeme));
    } else if(match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    } else if(match({TokenType::LEFT_PAREN})) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Esperado ) para fechar o agrupamento.");
        return expr;
    } else if(match({TokenType::LEFT_BRACKET})) {
        std::vector<std::unique_ptr<Expr>> elements;
        if(!check(TokenType::RIGHT_BRACKET)) {
            elements.emplace_back(std::move(expression()));
            while(match({TokenType::COMMA})) {
                elements.emplace_back(std::move(expression()));
            }
        }
        consume(TokenType::RIGHT_BRACKET, "Esperado ']'");
        return std::make_unique<ArrayLiteralExpr>(std::move(elements));
    } else if(check(TokenType::KW_CHAR) || check(TokenType::KW_INTEGER) || check(TokenType::KW_LOGIC) || check(TokenType::KW_REAL) || check(TokenType::KW_TEXT) || check(TokenType::KW_VECTOR)) {
        std::vector<Token> instType = type();
        consume(TokenType::LEFT_PAREN, "Esperado '(' após o tipo para instanciação.");
        
        std::unique_ptr<Expr> sizeExpr = expression();
        consume(TokenType::COMMA, "O construtor exige exatamente dois argumentos: (tamanho, valor_inicial).");
        std::unique_ptr<Expr> initExpr = expression();

        consume(TokenType::RIGHT_PAREN, "Esperado ')' após os argumentos.");
        return std::make_unique<InstantiateExpr>(instType, std::move(sizeExpr), std::move(initExpr));
    }
    throw error(peek(), "Expressão esperada.");
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();
    while(true) {
        if(match({TokenType::LEFT_BRACKET})) {
            std::unique_ptr<Expr> index = expression();
            consume(TokenType::RIGHT_BRACKET, "Esperado ']' após o índice.");
            expr = std::make_unique<IndexAccessExpr>(std::move(expr), std::move(index));
        } else if(match({TokenType::LEFT_PAREN})) {
            std::vector<std::unique_ptr<Expr>> args;
            if(!check(TokenType::RIGHT_PAREN)) {
                args.emplace_back(std::move(expression()));
                while(match({TokenType::COMMA})) {
                    args.emplace_back(std::move(expression()));
                }
            }
            consume(TokenType::RIGHT_PAREN, "Esperado ) para fechar a chamada de função.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        } else if(match({TokenType::DOT})) {
            Token name = consume(TokenType::IDENTIFIER, "Esperado o identificador do método após o '.'");
            consume(TokenType::LEFT_PAREN, "Esperado ( para abrir a chamada de função.");
            std::vector<std::unique_ptr<Expr>> args;
            if(!check(TokenType::RIGHT_PAREN)) {
                args.emplace_back(std::move(expression()));
                while(match({TokenType::COMMA})) {
                    args.emplace_back(std::move(expression()));
                }
            }
            consume(TokenType::RIGHT_PAREN, "Esperado ) para fechar a chamada de função.");
            expr = std::make_unique<MethodCallExpr>(std::move(expr), name, std::move(args));
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::postfix() {
    std::unique_ptr<Expr> leftExpr = call();
    if(match({TokenType::PLUS_PLUS, TokenType::MINUS_MINUS})) {
        Token op = previous();
        if(dynamic_cast<VariableExpr*>(leftExpr.get()) != nullptr || dynamic_cast<IndexAccessExpr*>(leftExpr.get()) != nullptr) {
            return std::make_unique<PrefixPostfixExpr>(std::move(leftExpr), op, false);
        }
        throw error(op, "Tentativa de incremento/decremento fora de variáveis.");
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::unary() {
    if(match({TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = unary();
        return std::make_unique<UnaryExpr>(op, std::move(rightExpr));
    } else if(match({TokenType::MINUS_MINUS, TokenType::PLUS_PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = unary();
        if(dynamic_cast<VariableExpr*>(rightExpr.get()) != nullptr || dynamic_cast<IndexAccessExpr*>(rightExpr.get()) != nullptr) {
            return std::make_unique<PrefixPostfixExpr>(std::move(rightExpr), op, true);
        }
        throw error(op, "Tentativa de pré-incremento/pré-decremento fora de variáveis.");
    }
    return postfix();
}

std::unique_ptr<Expr> Parser::exponent() {
    std::unique_ptr<Expr> leftExpr = unary();
    if(match({TokenType::POTENCY})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = exponent();
        leftExpr = std::make_unique<BinaryExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> leftExpr = exponent();
    while(match({TokenType::SLASH, TokenType::STAR, TokenType::MOD})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = exponent();
        leftExpr = std::make_unique<BinaryExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> leftExpr = factor();
    while(match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = factor();
        leftExpr = std::make_unique<BinaryExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> leftExpr = term();
    while(match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = term();
        leftExpr = std::make_unique<BinaryExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> leftExpr = comparison();
    while(match({TokenType::EQUAL_EQUAL, TokenType::NOT_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = comparison();
        leftExpr = std::make_unique<BinaryExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::logic_and() {
    std::unique_ptr<Expr> leftExpr = equality();
    while(match({TokenType::AND})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = equality();
        leftExpr = std::make_unique<LogicalExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::logic_or() {
    std::unique_ptr<Expr> leftExpr = logic_and();
    while(match({TokenType::OR})) {
        Token op = previous();
        std::unique_ptr<Expr> rightExpr = logic_and();
        leftExpr = std::make_unique<LogicalExpr>(std::move(leftExpr), op, std::move(rightExpr));
    }
    return leftExpr;
}

std::unique_ptr<Expr> Parser::ternary() {
    std::unique_ptr<Expr> expr = logic_or();
    if(match({TokenType::QUERY})) {
        std::unique_ptr<Expr> midExpr = expression();
        consume(TokenType::COLON, "Dois pontos esperado no operador ternário.");
        std::unique_ptr<Expr> rightExpr = ternary();
        return std::make_unique<TernaryExpr>(std::move(expr), std::move(midExpr), std::move(rightExpr));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = ternary();
    if(match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::SLASH_EQUAL, TokenType::STAR_EQUAL, TokenType::POTENCY_EQUAL, TokenType::MOD_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> value = assignment();
        if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(varExpr->name, op, std::move(value));
        } else if(IndexAccessExpr* indexAccessExpr = dynamic_cast<IndexAccessExpr*>(expr.get())) {
            return std::make_unique<IndexAssignExpr>(std::move(indexAccessExpr->target), std::move(indexAccessExpr->index), op, std::move(value));
        }

        throw error(op, "Atribuição inválida.");
    }
    return expr;
}

std::unique_ptr<Stmt> Parser::exprStmt() {
    return std::make_unique<ExpressionStmt>(std::move(expression()));
}

std::unique_ptr<Stmt> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while(!isAtEnd() && !check(TokenType::RIGHT_BRACE)) {
        if(match({TokenType::NEWLINE, TokenType::SEMICOLON})) continue;

        for(auto& st : declaration()) {
            statements.emplace_back(std::move(st));
        }
    }
    consume(TokenType::RIGHT_BRACE, "Esperado chave de fechamento '}'");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::ifStmt() {
    consume(TokenType::LEFT_PAREN, "Esperado '(' após o 'se'");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Esperado ')' após a condição do 'se'");

    consume(TokenType::LEFT_BRACE, "Esperado '{' após o 'se'");
    std::unique_ptr<Stmt> thenBranch = block();

    std::unique_ptr<Stmt> elseBranch = nullptr;
    if(match({TokenType::ELSE})) {
        if(match({TokenType::IF})) {
            elseBranch = ifStmt();
        } else {
            consume(TokenType::LEFT_BRACE, "Esperado '{' após o 'senao'");
            elseBranch = block();
        }
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStmt() {
    consume(TokenType::LEFT_PAREN, "Esperado '(' após o 'enquanto'");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Esperado ')' após a condição do 'enquanto'");

    consume(TokenType::LEFT_BRACE, "Esperado '{' após o 'enquanto'");
    std::unique_ptr<Stmt> body = block();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::forStmt() {
    consume(TokenType::LEFT_PAREN, "Esperado '(' após o 'para'");
    
    std::unique_ptr<Stmt> initializer;
    if(match({TokenType::SEMICOLON})) {
        initializer = nullptr;
    } else if(check(TokenType::KW_CHAR) || check(TokenType::KW_INTEGER) || check(TokenType::KW_LOGIC) || check(TokenType::KW_REAL) || check(TokenType::KW_TEXT) || check(TokenType::KW_VECTOR)) {
        std::vector<Token> declType = type();
        Token name = consume(TokenType::IDENTIFIER, "Nome da variável esperado.");
        std::vector<std::unique_ptr<Stmt>> declarations = varDecl(declType, name);
        initializer = std::make_unique<BlockStmt>(std::move(declarations));
        consume(TokenType::SEMICOLON, "Esperado ';' após a inicialização do 'para'");
    } else {
        initializer = exprStmt();
        consume(TokenType::SEMICOLON, "Esperado ';' após a inicialização do 'para'");
    }

    std::unique_ptr<Expr> condition = nullptr;
    if(!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Esperado ';' após a condição do 'para'");

    std::unique_ptr<Expr> increment = nullptr;
    if(!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Esperado ')' após o 'para'");

    consume(TokenType::LEFT_BRACE, "Esperado '{' após o 'para'");
    std::unique_ptr<Stmt> body = block();

    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body));
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

std::vector<Token> Parser::type() {
    std::vector<Token> types;
    if(match({TokenType::KW_CHAR, TokenType::KW_INTEGER, TokenType::KW_TEXT, TokenType::KW_REAL, TokenType::KW_LOGIC, TokenType::KW_VOID})) {
        types.emplace_back(previous());
    } else if(match({TokenType::KW_VECTOR})) {
        types.emplace_back(previous());
        consume(TokenType::LESS, "Esperado '<' para abrir a definição do tipo do vetor.");
        for(Token ty : type()) {
            types.emplace_back(ty);
        }
        consume(TokenType::GREATER, "Esperado '>' para fechar a definição do tipo do vetor.");
    }

    if(types.empty()) {
        throw error(peek(), "Tipo de dado inválido ou não reconhecido.");
    }

    return types;
}

std::vector<std::unique_ptr<Stmt>> Parser::constDecl() {
    std::vector<std::unique_ptr<Stmt>> declarations;
    consume(TokenType::KW_CONST, "Esperado 'constante' no início da declaração de uma constante.");
    std::vector<Token> declType = type();
    
    auto parseConstInit = [&]() -> std::unique_ptr<Expr> {
        if (match({TokenType::EQUAL})) {
            return expression();
        } else if (match({TokenType::LEFT_PAREN})) {
            std::unique_ptr<Expr> sizeExpr = expression();
            consume(TokenType::COMMA, "O construtor exige exatamente dois argumentos: (tamanho, valor_inicial).");
            std::unique_ptr<Expr> initExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Esperado ')' para fechar os argumentos.");
            return std::make_unique<InstantiateExpr>(declType, std::move(sizeExpr), std::move(initExpr));
        }
        throw error(peek(), "Constantes precisam ser inicializadas.");
    };

    Token name = consume(TokenType::IDENTIFIER, "Nome da constante esperado.");
    declarations.emplace_back(std::make_unique<VarDeclStmt>(declType, name, true, parseConstInit()));
    
    while(match({TokenType::COMMA})) {
        Token nextName = consume(TokenType::IDENTIFIER, "Nome da constante esperado.");
        declarations.emplace_back(std::make_unique<VarDeclStmt>(declType, nextName, true, parseConstInit()));
    }
    
    return declarations;
}

std::vector<std::unique_ptr<Stmt>> Parser::varDecl(std::vector<Token> declType, Token name) {
    std::vector<std::unique_ptr<Stmt>> declarations;

    auto parseVarInit = [&]() -> std::unique_ptr<Expr> {
        if(match({TokenType::EQUAL})) {
            return expression();
        } else if(match({TokenType::LEFT_PAREN})) {
            std::unique_ptr<Expr> sizeExpr = expression();
            consume(TokenType::COMMA, "O construtor exige exatamente dois argumentos: (tamanho, valor_inicial).");
            std::unique_ptr<Expr> initExpr = expression();
            consume(TokenType::RIGHT_PAREN, "Esperado ')' para fechar os argumentos.");
            return std::make_unique<InstantiateExpr>(declType, std::move(sizeExpr), std::move(initExpr));
        } else {
            if(declType.size() == 1) {
                std::any defaultValue;
                if(declType[0].type == TokenType::KW_INTEGER || declType[0].type == TokenType::KW_REAL) {
                    defaultValue = declType[0].type == TokenType::KW_INTEGER ? 0LL : 0.0;
                } else if(declType[0].type == TokenType::KW_LOGIC) {
                    defaultValue = false;
                } else if(declType[0].type == TokenType::KW_CHAR) {
                    defaultValue = '\0';
                } else if(declType[0].type == TokenType::KW_TEXT) {
                    defaultValue = std::string("");
                }
                return std::make_unique<LiteralExpr>(defaultValue);
            } else {
                return std::make_unique<ArrayLiteralExpr>(std::vector<std::unique_ptr<Expr>>());
            }
        }
    };

    declarations.emplace_back(std::make_unique<VarDeclStmt>(declType, name, false, parseVarInit()));

    while(match({TokenType::COMMA})) {
        Token nextName = consume(TokenType::IDENTIFIER, "Nome da variável esperado.");
        declarations.emplace_back(std::make_unique<VarDeclStmt>(declType, nextName, false, parseVarInit()));
    }

    return declarations;
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

        bool isFunction = false;
        if(check(TokenType::LEFT_PAREN)) {
            TokenType nextType = tokens[current + 1].type; 
            if(nextType == TokenType::RIGHT_PAREN || nextType == TokenType::KW_CHAR || nextType == TokenType::KW_INTEGER || nextType == TokenType::KW_LOGIC || nextType == TokenType::KW_REAL || nextType == TokenType::KW_TEXT || nextType == TokenType::KW_VECTOR) {
                isFunction = true;
            }
        }

        if(isFunction) {
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
        if(match({TokenType::NEWLINE, TokenType::SEMICOLON})) continue;

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