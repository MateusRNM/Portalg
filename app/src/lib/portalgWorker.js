let interpreter = null;

async function initInterpreter() {
    const dynamicImport = new Function('path', 'return import(path)');
    const moduloInit = await dynamicImport('/interpreter/portalg.js');
    interpreter = await moduloInit.default();
}

initInterpreter();

self.onmessage = (event) => {
    const { command, payload } = event.data;

    if(!interpreter) return;

    switch(command) {
        case 'RUN_CODE':
            const { code, debugMode } = payload;
            const result = interpreter.runCode(code, debugMode);

            if(!result.success) {
                postMessage({ type: 'ERROR', line: result.lineError, message: result.messsage });
            } else {
                postMessage({ type: 'EXECUTION_FINISHED' });
            }
            break;
        case 'INPUT_RESPONSE':
        case 'DEBUG_NEXT_STEP':
            break;
    }
}