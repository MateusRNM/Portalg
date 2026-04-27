#pragma once
#include <string>
#include <vector>

struct Error {
    int line;
    int column;
    std::string message;
};

class ErrorHandler {
    private:
        std::vector<Error> errors;
    public:
        void error(int line, int column, std::string message);
        bool haveErrors();
        void report();
};