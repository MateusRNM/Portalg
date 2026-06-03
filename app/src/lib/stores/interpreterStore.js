import { writable } from 'svelte/store';

export const terminalOutput = writable("");
export const isWaitingInput = writable(false);
export const debugState = writable(null); 
export const debugLine = writable(0);

let worker;

export function initWorker() {
    if (typeof window !== 'undefined' && !worker) {
        worker = new Worker(new URL('../portalgWorker.js', import.meta.url), { type: 'module' });

        worker.onmessage = (event) => {
            const data = event.data;
            
            switch (data.type) {
                case 'WRITE':
                    terminalOutput.update(text => text + data.text);
                    break;
                case 'INPUT':
                    isWaitingInput.set(true);
                    break;
                case 'DEBUG_STATE':
                    debugState.set(JSON.parse(data.state));
                    debugLine.set(data.line);
                    break;
                case 'EXECUTION_ERROR':
                    terminalOutput.update(text => text + `\n[Erro Linha ${data.line}]: ${data.message}`);
                    break;
            }
        };
    }
}

export function executeCode(code, debugMode) {
    terminalOutput.set(""); 
    debugState.set(null);
    worker.postMessage({ command: 'RUN_CODE', payload: { code, debugMode } });
}

export function sendInputResponse(text) {
    isWaitingInput.set(false);
    terminalOutput.update(t => t + text + '\n');
    worker.postMessage({ command: 'INPUT_RESPONSE', payload: { text } });
}

export function sendNextStep() {
    worker.postMessage({ command: 'DEBUG_NEXT_STEP' });
}