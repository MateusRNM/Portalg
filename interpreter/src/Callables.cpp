#include "Callables.h"
#include "Interpreter.h"
#include <iostream>

std::any NativeEscreva::call(Interpreter *interpreter, const std::vector<std::any> &arguments) {
    for (const auto &arg : arguments) {
        std::cout << interpreter->stringify(arg);
    }
    return {};
}

std::any NativeEscreval::call(Interpreter *interpreter, const std::vector<std::any> &arguments) {
    for (const auto &arg : arguments) {
        std::cout << interpreter->stringify(arg);
    }
    std::cout << '\n';
    return {};
}

std::any NativeLeia::call(Interpreter *interpreter, const std::vector<std::any> &arguments) {
    std::string input;
    std::getline(std::cin, input);
    return input;
}