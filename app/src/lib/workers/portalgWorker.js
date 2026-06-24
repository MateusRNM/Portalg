let interpreter = null;

async function initInterpreter() {
    if(!interpreter) {
        const module = await import('../../../static/interpreter/portalg.js');
        interpreter = await module.default();
    }
}

self.onmessage = async (event) => {
    const { command, payload } = event.data;
    await initInterpreter();

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