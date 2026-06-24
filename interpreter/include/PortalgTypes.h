#pragma once
#include <variant>
#include <memory>
#include <string>
#include <vector>
#include "Token.h"

class PortalgCallable;
class Environment;

using PortalgValue = std::variant<
    std::monostate,                      
    long long,                           
    double,                             
    bool,                               
    char,                               
    std::string,                         
    std::shared_ptr<struct TypedArray>,
    std::shared_ptr<PortalgCallable>,
    std::shared_ptr<struct PortalgRef>
>;

struct TypedArray {
    std::shared_ptr<std::vector<Token>> typeTokens;
    std::shared_ptr<std::vector<PortalgValue>> elements; 
};

struct PortalgRef {
    std::shared_ptr<Environment> env;
    std::string name;
};