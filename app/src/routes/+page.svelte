<script>
    import { onMount } from 'svelte';
    import { 
        initWorker, executeCode, sendInputResponse, sendNextStep, 
        terminalOutput, isWaitingInput, debugState, debugLine 
    } from '$lib/stores/interpreterStore';

    let code = $state("inteiro x = 5\nescreval(\"O valor é: \", x)\nleia()");
    let userInput = $state("");
    let isDebugMode = $state(false);

    onMount(() => {
        initWorker();
    });

    function handleInputKeydown(event) {
        if (event.key === 'Enter') {
            sendInputResponse(userInput);
            userInput = "";
        }
    }
</script>

<div class="container">
    <div class="editor-section">
        <h2>Editor Portalg</h2>
        <textarea bind:value={code} rows="10" cols="50"></textarea>
        <br>
        <label>
            <input type="checkbox" bind:checked={isDebugMode}> Modo Debug (Passo a Passo)
        </label>
        <button onclick={() => executeCode(code, isDebugMode)}>Executar</button>
    </div>

    {#if $debugState}
        <div class="debug-panel" style="background: #222; color: #0f0; padding: 10px; margin-top: 10px;">
            <h3>Debugger (Pausado na linha {$debugLine})</h3>
            <pre>{JSON.stringify($debugState, null, 2)}</pre>
            <button onclick={sendNextStep}>Próxima Linha ⏭️</button>
        </div>
    {/if}

    <div class="terminal" style="background: #000; color: #fff; padding: 10px; margin-top: 10px; min-height: 150px;">
        <pre style="margin: 0;">{$terminalOutput}</pre>
        
        {#if $isWaitingInput}
            <div style="display: flex; align-items: center; margin-top: 5px;">
                <span style="margin-right: 5px;">></span>
                <input
                    type="text" 
                    bind:value={userInput} 
                    onkeydown={handleInputKeydown}
                    style="background: transparent; color: white; border: none; outline: none; width: 100%;"
                />
            </div>
        {/if}
    </div>
</div>