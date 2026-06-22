let interpreter = null;

async function initInterpreter() {
    const dynamicImport = new Function('path', 'return import(path)');
    const moduloInit = await dynamicImport('/interpreter/portalg.js');
    interpreter = await moduloInit.default();
}

initInterpreter();

self.onmessage = async (event) => {
    const { command, payload } = event.data;

    if(!interpreter) return;

    let result;
    switch(command) {
        case 'RUN_CODE':
            result = await interpreter.runCode(payload.code, payload.debugMode);
            if(!result.success) {
                postMessage({ type: 'EXECUTION_ERROR', line: result.lineError, error_message: result.messsage });
            } else {
                postMessage({ type: 'EXECUTION_FINISHED' });
            }
            break;
        case 'INPUT_RESPONSE':
        case 'DEBUG_NEXT_STEP':
            break;
    }
}