#include <gtest/gtest.h>
#include <iostream>
#include "Scanner.h"
#include "ErrorHandler.h"

TEST(ScannerTest, RecognizeIntegerKeyword) {
    std::string code = "// =\n [] /* {{{{{}}}}} */ ==";
    ErrorHandler errorHandler = ErrorHandler();
    Scanner scanner(code, errorHandler);
    
    std::vector<Token> tokens = scanner.scanTokens();

    for(Token t : tokens) {
        std::cout << t.lexeme << '\n';
    }
    
    // ASSERT_EQ(tokens.size(), 9);
    // EXPECT_EQ(tokens[0].type, TokenType::LEFT_PAREN);
    // EXPECT_EQ(tokens[2].type, TokenType::LEFT_BRACKET);
    // EXPECT_EQ(tokens[6].type, TokenType::RIGHT_BRACE);
    // EXPECT_EQ(tokens[8].type, TokenType::EOF_TOKEN);
}