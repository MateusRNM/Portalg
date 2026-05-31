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

std::any NativeRaiz::call(Interpreter *interpreter, const std::vector<std::any>& arguments) {
    if(arguments.size() == 0 || arguments.size() > 2) {
        throw std::runtime_error("A função nativa 'raiz' espera 1 ou 2 argumentos.");
    }

    double radicand = 0.0;
    if(arguments[0].type() == typeid(double)) {
        radicand = std::any_cast<double>(arguments[0]);
    } else if(arguments[0].type() == typeid(long long)) {
        radicand = (double)std::any_cast<long long>(arguments[0]);
    } else {
        throw std::runtime_error("O radicando da função 'raiz' deve ser numérico.");
    }

    if(radicand < 0) {
        throw std::runtime_error("Não existem raízes reais para números negativos.");
    }

    long long degree = 2;
    if(arguments.size() == 2) {
        if(arguments[1].type() != typeid(long long)) {
            throw std::runtime_error("O índice da função 'raiz' deve ser um número inteiro.");
        }
        degree = std::any_cast<long long>(arguments[1]);
    }

    if(degree <= 0) {
        throw std::runtime_error("O índice da função 'raiz' deve ser um número inteiro positivo.");
    }

    return std::pow(radicand, 1.0 / (double)degree);
}

std::any NativeLog::call(Interpreter *interpreter, const std::vector<std::any>& arguments) {
    double logaritm = 0.0;
    if(arguments[0].type() == typeid(double)) {
        logaritm = std::any_cast<double>(arguments[0]);
    } else if(arguments[0].type() == typeid(long long)) {
        logaritm = (double)std::any_cast<long long>(arguments[0]);
    } else {
        throw std::runtime_error("O logaritmando da função 'log' deve ser numérico.");
    }

    if(logaritm <= 0) {
        throw std::runtime_error("O logaritmando da função 'log' deve ser um número positivo.");
    }

    double base = 0.0;
    if(arguments[1].type() == typeid(double)) {
        base = std::any_cast<double>(arguments[1]);
    } else if(arguments[1].type() == typeid(long long)) {
        base = (double)std::any_cast<long long>(arguments[1]);
    } else {
        throw std::runtime_error("A base da função 'log' deve ser numérica.");
    }

    if(base <= 0 || base == 1) {
        throw std::runtime_error("A base da função 'log' deve ser um número positivo diferente de 1.");
    }

    return std::log(logaritm) / std::log(base);
}

std::any NativeArredondaCima::call(Interpreter *interpreter, const std::vector<std::any>& arguments) {
    double value = 0.0;
    if(arguments[0].type() == typeid(double)) {
        value = std::any_cast<double>(arguments[0]);
    } else if(arguments[0].type() == typeid(long long)) {
        value = (double)std::any_cast<long long>(arguments[0]);
    } else {
        throw std::runtime_error("O argumento da função 'arredonda_cima' deve ser numérico.");
    }

    return std::ceil(value);
}

std::any NativeArredondaBaixo::call(Interpreter *interpreter, const std::vector<std::any>& arguments) {
    double value = 0.0;
    if(arguments[0].type() == typeid(double)) {
        value = std::any_cast<double>(arguments[0]);
    } else if(arguments[0].type() == typeid(long long)) {
        value = (double)std::any_cast<long long>(arguments[0]);
    } else {
        throw std::runtime_error("O argumento da função 'arredonda_baixo' deve ser numérico.");
    }
    
    return std::floor(value);
}