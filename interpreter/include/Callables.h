#pragma once
#include <iostream>
#include <vector>
#include <any>

class Interpreter;

class PortalgCallable {
public:
    virtual ~PortalgCallable() = default;
    virtual int arity() = 0;
    virtual std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) = 0;
};

struct NativeEscreva : public PortalgCallable {
    int arity() override { return -1; }
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};

struct NativeEscreval : public PortalgCallable {
    int arity() override { return -1; }
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};

struct NativeLeia : public PortalgCallable {
    int arity() override { return 0; }
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};

struct NativeRaiz : public PortalgCallable {
    int arity() override { return -1; };
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};

struct NativeLog : public PortalgCallable {
    int arity() override { return 2; };
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};

struct NativeArredondaCima : public PortalgCallable {
    int arity() override { return 1; };
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};

struct NativeArredondaBaixo : public PortalgCallable {
    int arity() override { return 1; };
    std::any call(Interpreter *interpreter, const std::vector<std::any> &arguments) override;
};