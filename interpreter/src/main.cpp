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

class ASTPrinter : public ExprVisitor {
public:
    std::string print(Expr* expr) {
        if (expr == nullptr) return "nulo";
        return std::any_cast<std::string>(expr->accept(this));
    }

private:
    std::string parenthesize(std::string name, std::vector<Expr*> exprs) {
        std::string result = "(" + name;
        for (Expr* expr : exprs) {
            result += " " + print(expr);
        }
        result += ")";
        return result;
    }

    std::string parenthesizeStr(std::string name, std::vector<std::string> parts) {
        std::string result = "(" + name;
        for (const std::string& part : parts) {
            result += " " + part;
        }
        result += ")";
        return result;
    }

public:
    std::any visitBinaryExpr(BinaryExpr* expr) override {
        return parenthesize(expr->op.lexeme, {expr->left.get(), expr->right.get()});
    }

    std::any visitLogicalExpr(LogicalExpr* expr) override {
        return parenthesize(expr->op.lexeme, {expr->left.get(), expr->right.get()});
    }

    std::any visitUnaryExpr(UnaryExpr* expr) override {
        return parenthesize(expr->op.lexeme, {expr->right.get()});
    }

    std::any visitPrefixPostfixExpr(PrefixPostfixExpr* expr) override {
        std::string name = expr->isPrefix ? "pre_" : "pos_";
        return parenthesize(name + expr->op.lexeme, {expr->target.get()});
    }

    std::any visitTernaryExpr(TernaryExpr* expr) override {
        return parenthesize("?", {expr->condition.get(), expr->trueExpr.get(), expr->falseExpr.get()});
    }

    std::any visitAssignExpr(AssignExpr* expr) override {
        return parenthesize(expr->op.lexeme + " " + expr->name.lexeme, {expr->value.get()});
    }

    std::any visitIndexAssignExpr(IndexAssignExpr* expr) override {
        return parenthesize(expr->op.lexeme + "_index", {expr->target.get(), expr->index.get(), expr->value.get()});
    }

    std::any visitCallExpr(CallExpr* expr) override {
        std::string result = "(chamada " + print(expr->callee.get());
        for (auto& arg : expr->arguments) {
            result += " " + print(arg.get());
        }
        result += ")";
        return result;
    }

    std::any visitMethodCallExpr(MethodCallExpr* expr) override {
        std::string result = "(metodo " + print(expr->object.get()) + " ." + expr->methodName.lexeme;
        for (auto& arg : expr->arguments) {
            result += " " + print(arg.get());
        }
        result += ")";
        return result;
    }

    std::any visitIndexAccessExpr(IndexAccessExpr* expr) override {
        return parenthesize("acesso_vetor", {expr->target.get(), expr->index.get()});
    }

    std::any visitArrayLiteralExpr(ArrayLiteralExpr* expr) override {
        std::string result = "(vetor";
        for (auto& el : expr->elements) {
            result += " " + print(el.get());
        }
        result += ")";
        return result;
    }

    std::any visitInstantiateExpr(InstantiateExpr* expr) override {
        return parenthesize("instanciar_" + expr->type[0].lexeme, {expr->size.get(), expr->initialValue.get()});
    }

    std::any visitVariableExpr(VariableExpr* expr) override {
        return expr->name.lexeme;
    }

    std::any visitLiteralExpr(LiteralExpr* expr) override {
        if (!expr->value.has_value()) return std::string("nulo");
        
        if (expr->value.type() == typeid(double)) return std::to_string(std::any_cast<double>(expr->value));
        if (expr->value.type() == typeid(long long)) return std::to_string(std::any_cast<long long>(expr->value));
        if (expr->value.type() == typeid(std::string)) return "\"" + std::any_cast<std::string>(expr->value) + "\"";
        if (expr->value.type() == typeid(char)) return std::string("'") + std::any_cast<char>(expr->value) + std::string("'");
        if (expr->value.type() == typeid(bool)) return std::any_cast<bool>(expr->value) ? std::string("verdadeiro") : std::string("falso");
        
        return std::string("desconhecido");
    }
};

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_INTEGER: return "KW_INTEGER";
        case TokenType::KW_REAL: return "KW_REAL";
        case TokenType::KW_CHAR: return "KW_CHAR";
        case TokenType::KW_LOGIC: return "KW_LOGIC";
        case TokenType::KW_TEXT: return "KW_TEXT";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::LITERAL_INTEGER: return "LITERAL_INTEGER";
        case TokenType::LITERAL_REAL: return "LITERAL_REAL";
        case TokenType::LITERAL_TEXT: return "LITERAL_TEXT";
        case TokenType::LITERAL_CHAR: return "LITERAL_CHAR";
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

    ASTPrinter printer;

    for(auto& node : ast) {
        if(ExpressionStmt* expr = dynamic_cast<ExpressionStmt*>(node.get())) {
            std::cout << printer.print(expr->expression.get()) << '\n';
        } 

        else if(VarDeclStmt* varDecl = dynamic_cast<VarDeclStmt*>(node.get())) {
            std::string tipoDecl = varDecl->isConst ? "(constante " : "(variavel ";
            
            std::string nomeTipo = "";
            for(Token t : varDecl->type) nomeTipo += t.lexeme;
            
            std::cout << tipoDecl << nomeTipo << " " << varDecl->name.lexeme;
            
            if (varDecl->initializer) {
                std::cout << " " << printer.print(varDecl->initializer.get());
            }
            
            std::cout << ")\n";
        }
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