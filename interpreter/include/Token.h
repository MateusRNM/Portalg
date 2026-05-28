#pragma once
#include <string>

enum class TokenType {
    LEFT_PAREN, RIGHT_PAREN,       
    LEFT_BRACE, RIGHT_BRACE,       
    LEFT_BRACKET, RIGHT_BRACKET,   
    COMMA, DOT, COLON, SEMICOLON, QUERY,     
    AMPERSAND,

    MINUS, PLUS, SLASH, STAR, MOD,
    POTENCY,          
    PLUS_PLUS, MINUS_MINUS,       

    EQUAL,                     
    EQUAL_EQUAL, NOT_EQUAL,        
    GREATER, GREATER_EQUAL,    
    LESS, LESS_EQUAL,             
    PLUS_EQUAL, MINUS_EQUAL,      
    SLASH_EQUAL, STAR_EQUAL,       
    POTENCY_EQUAL, MOD_EQUAL,     

    AND, OR, NOT,             

    KW_INTEGER, KW_REAL, KW_TEXT, KW_CHAR, 
    KW_LOGIC, KW_VECTOR, KW_VOID, KW_CONST,

    IF, ELSE,                     
    SWITCH, CASE, DEFAULT_CASE,   
    WHILE, FOR,                    
    BREAK, CONTINUE,            
    RETURN,

    IDENTIFIER,                   
    LITERAL_INTEGER,          
    LITERAL_REAL,               
    LITERAL_TEXT,            
    LITERAL_CHAR,                  
    LITERAL_TRUE, LITERAL_FALSE,   

    NEWLINE, EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};