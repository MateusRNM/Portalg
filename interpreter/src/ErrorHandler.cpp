#include "ErrorHandler.h"

void ErrorHandler::report(int line, int column, std::string message) {
    hadError = true;
    std::cout << "[ ERROR - line " << line << " column " << column << "] " << message;
}

void ErrorHandler::error(int line, int column, std::string message) {
    report(line, column, message);
}