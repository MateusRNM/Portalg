#pragma once
#include <string>

class ErrorHandler {
    private:
        void report(int line, int column, std::string message);
    public:
        bool hadError = false;
        void error(int line, int column, std::string message);
};