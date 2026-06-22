#include "ErrorHandler.h"
#include "Scanner.h"
#include "Parser.h"
#include "Resolver.h"
#include "Interpreter.h"

struct ExecutionResult {
    bool success;
    std::string message;
    int lineError;
};

void report_portalg(ErrorHandler& errorHandler, ExecutionResult& result) {
    std::vector<Error> errors = errorHandler.get_errors();
    result.success = false;
    for(size_t i = 0; i < errors.size(); i++) {
        result.message += "[ERRO " + std::to_string(i+1) + " Linha " + std::to_string(errors[i].line) + " | Coluna " + std::to_string(errors[i].column )+ "] " + errors[i].message + "\n";
    }
    result.lineError = errors[0].line;
}

#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>
using namespace emscripten;

ExecutionResult runCode(std::string code, bool debugMode) {
    ExecutionResult result = {true, "", 0};

    try {
        ErrorHandler errorHandler;

        Scanner scanner(code, errorHandler);
        std::vector<Token> tokens = scanner.scanTokens();

        if(errorHandler.haveErrors()) {
            report_portalg(errorHandler, result);
            return result;
        }

        Parser parser(tokens, errorHandler);
        std::vector<std::unique_ptr<Stmt>> ast = parser.parse();

        if(errorHandler.haveErrors()) {
            report_portalg(errorHandler, result);
            return result;
        }

        Interpreter interpreter;
        interpreter.debugModeOn = debugMode;

        Resolver resolver(&interpreter);
        
        resolver.resolve(ast);
        interpreter.interpret(ast);
    } catch (const RuntimeError& e) {
        result.success = false;
        result.message = e.what();
        result.lineError = e.token.line;
    } catch (const std::exception& e) {
        result.success = false;
        result.message = e.what();
        result.lineError = 0;
    } catch (...) {
        result.success = false;
        result.message = "Erro interno.";
        result.lineError = 0;
    }
    return result;
}

EMSCRIPTEN_BINDINGS(portalg_module) {
    value_object<ExecutionResult>("ExecutionResult")
        .field("success", &ExecutionResult::success)
        .field("message", &ExecutionResult::message)
        .field("lineError", &ExecutionResult::lineError);
    function("runCode", &runCode);
}

#endif