#include "Callables.h"
#include "Interpreter.h"
#include <iostream>
#include <random>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstdlib>

EM_JS(void, write_in_terminal, (const char* text), {
    postMessage({ type: "WRITE", text: UTF8ToString(text) });
});

EM_ASYNC_JS(char*, input_in_terminal, (), {
    return await new Promise((resolve) => {
        postMessage({ type: "INPUT" });

        const listener = (event) => {
            if(event.data.command === "INPUT_RESPONSE") {
                self.removeEventListener("message", listener);
                const text = event.data.payload.text;
                const byteCount = lengthBytesUTF8(text) + 1;
                const pointer = _malloc(byteCount);
                stringToUTF8(text, pointer, byteCount);
                resolve(pointer);
            }
        };
        self.addEventListener("message", listener);
    });
});

#endif

std::any NativeEscreva::call(Interpreter *interpreter, const std::vector<std::any> &arguments) {
    for (const auto &arg : arguments) {
        std::string text = interpreter->stringify(arg);
        #ifdef __EMSCRIPTEN__
            write_in_terminal(text.c_str());
        #else
            std::cout << text;
        #endif
    }
    return {};
}

std::any NativeEscreval::call(Interpreter *interpreter, const std::vector<std::any> &arguments) {
    for (const auto &arg : arguments) {
        std::string text = interpreter->stringify(arg);
        #ifdef __EMSCRIPTEN__
            write_in_terminal(text.c_str());
        #else
            std::cout << text;
        #endif
    }

    #ifdef __EMSCRIPTEN__
        write_in_terminal("\n");
    #else
        std::cout << '\n';
    #endif
    return {};
}

std::any NativeLeia::call(Interpreter *interpreter, const std::vector<std::any> &arguments) {
    #ifdef __EMSCRIPTEN__
        char* pointerText = input_in_terminal();
        std::string input(pointerText);
        free(pointerText);
        return input;
    #else
        std::string input;
        std::getline(std::cin, input);
        return input;
    #endif
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

std::any NativeAleatorio::call(Interpreter *interpreter, const std::vector<std::any>& arguments) {
    double min = 0.0;
    if(arguments[0].type() == typeid(double)) {
        min = std::any_cast<double>(arguments[0]);
    } else if(arguments[0].type() == typeid(long long)) {
        min = (double)std::any_cast<long long>(arguments[0]);
    } else {
        throw std::runtime_error("O primeiro argumento da função 'aleatorio' deve ser numérico.");
    }
    
    double max = 0.0;
    if(arguments[1].type() == typeid(double)) {
        max = std::any_cast<double>(arguments[1]);
    } else if(arguments[1].type() == typeid(long long)) {
        max = (double)std::any_cast<long long>(arguments[1]);
    } else {
        throw std::runtime_error("O segundo argumento da função 'aleatorio' deve ser numérico.");
    }

    if(min > max) {
        throw std::runtime_error("O início do intervalo da função 'aleatorio' deve ser menor ou igual ao fim do intervalo.");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max + 0.1);
    double result = dis(gen);
    if(result > max) result = max;
    
    return result;
}