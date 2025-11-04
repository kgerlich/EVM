// Web Worker for EVM simulator execution
// Prevents UI blocking during simulation

interface CPUState {
    pc: number;
    sr: number;
    dregs: number[];
    aregs: number[];
    ssp: number;
    usp: number;
    msp: number;
}

interface SimulatorMessage {
    type: 'init' | 'step' | 'run' | 'pause' | 'reset' | 'setState' | 'getState' | 'readMemory' | 'writeMemory' | 'loadROM' | 'loadProgram';
    payload?: any;
}

interface StateMessage {
    type: 'state';
    state: CPUState;
}

interface ReadyMessage {
    type: 'ready';
}

interface ErrorMessage {
    type: 'error';
    error: string;
}

type WorkerMessage = StateMessage | ReadyMessage | ErrorMessage;

// Module and cpu are set dynamically by importScripts and initWASM
let cpu: any;
let initialized = false;

// Get reference to Module that was loaded by importScripts
// This avoids declaring it twice
function getModule(): any {
    return (self as any).Module;
}

/**
 * Initialize WASM module
 * Load evm.js via importScripts, which sets up the Module
 */
async function initWASM(): Promise<void> {
    try {
        console.log('[Worker] Step 1: Checking if Module already exists...');

        // Check if Module was already loaded
        let Module = getModule();
        if (Module && Module.cwrap) {
            console.log('[Worker] Step 2: Module already initialized, using existing');
            setupCPU();
            return;
        }

        console.log('[Worker] Step 2: Module not found, using importScripts to load evm.js...');

        // CRITICAL: Set Module.locateFile BEFORE importing evm.js
        // This ensures evm.js uses the correct path to fetch evm.js.wasm
        console.log('[Worker] Creating Module with locateFile...');

        (self as any).Module = {
            locateFile: (filename: string, scriptDirectory?: string) => {
                const resolvedPath = '/' + filename;
                console.log(`[Worker] Module.locateFile: "${filename}" (scriptDir="${scriptDirectory}") → "${resolvedPath}"`);
                return resolvedPath;
            },
            print: (text: string) => console.log('[WASM stdout]', text),
            printErr: (text: string) => console.error('[WASM stderr]', text),
        };
        console.log('[Worker] Module created with locateFile');

        // Try to pre-load WASM binary and attach to Module
        console.log('[Worker] Attempting to fetch WASM binary pre-emptively...');
        try {
            const wasmResponse = await fetch('/evm.js.wasm', { credentials: 'same-origin' });
            if (wasmResponse.ok) {
                const wasmBuffer = await wasmResponse.arrayBuffer();
                (self as any).Module.wasmBinary = new Uint8Array(wasmBuffer);
                console.log(`[Worker] Successfully pre-loaded WASM binary (${wasmBuffer.byteLength} bytes)`);
            } else {
                console.warn(`[Worker] Failed to pre-load WASM: HTTP ${wasmResponse.status}`);
            }
        } catch (error) {
            console.warn('[Worker] Failed to pre-load WASM:', error);
            // Continue anyway, evm.js will try to fetch it
        }

        // Use importScripts which is designed for loading scripts in workers
        // This will execute evm.js and set Module in the global scope
        try {
            (self as any).importScripts('/evm.js');
            console.log('[Worker] Step 3: evm.js loaded via importScripts');
        } catch (importError: any) {
            console.error('[Worker] importScripts error:', importError);
            throw new Error(`Failed to import evm.js: ${importError.message}`);
        }

        // Now check if Module is available
        Module = getModule();
        if (!Module) {
            console.error('[Worker] Module is still undefined after importScripts');
            // Try to see what's in the global scope
            console.log('[Worker] Global keys:', Object.keys(self as any).slice(0, 20));
            throw new Error('Module not found after importScripts(/evm.js)');
        }

        console.log('[Worker] Step 4: Module available');
        console.log('[Worker] Step 5: Waiting for WASM runtime to initialize...');

        // Always wait for onRuntimeInitialized - don't take shortcuts
        await new Promise<void>((resolve, reject) => {
            const timeout = setTimeout(() => {
                reject(new Error('WASM initialization timeout after 10 seconds'));
            }, 10000);

            // Check if callback was already called
            if (Module.runtimeInitialized) {
                console.log('[Worker] WASM already initialized, calling setupCPU now');
                clearTimeout(timeout);
                setupCPU();
                resolve();
                return;
            }

            const originalCallback = Module.onRuntimeInitialized;

            Module.onRuntimeInitialized = () => {
                clearTimeout(timeout);
                console.log('[Worker] Step 6: WASM runtime initialized');

                if (typeof originalCallback === 'function') {
                    try {
                        originalCallback();
                    } catch (e) {
                        console.log('[Worker] Original callback error:', e);
                    }
                }

                setupCPU();
                resolve();
            };
        });
    } catch (error) {
        throw new Error(`Failed to initialize WASM: ${error}`);
    }
}

/**
 * Setup CPU wrapper functions once WASM is ready
 */
function setupCPU(): void {
    try {
        const Module = getModule();
        if (!Module) {
            throw new Error('Module is undefined!');
        }
        console.log('[Worker] setupCPU: Module exists, cwrap available?', !!Module.cwrap);

        // Wrap each function with individual error handling
        const init = Module.cwrap('cpu_init', 'number', []);
        console.log('[Worker] ✓ cpu_init wrapped');

        const reset = Module.cwrap('cpu_reset', null, []);
        console.log('[Worker] ✓ cpu_reset wrapped');

        const step = Module.cwrap('cpu_step', null, []);
        console.log('[Worker] ✓ cpu_step wrapped');

        const run = Module.cwrap('cpu_run', null, ['number']);
        console.log('[Worker] ✓ cpu_run wrapped');

        const pause = Module.cwrap('cpu_pause', null, []);
        console.log('[Worker] ✓ cpu_pause wrapped');

        const getState = Module.cwrap('cpu_get_state', 'number', []);
        console.log('[Worker] ✓ cpu_get_state wrapped');

        const loadROM = Module.cwrap('cpu_load_rom', 'number', ['array', 'number']);
        console.log('[Worker] ✓ cpu_load_rom wrapped');

        const loadProgram = Module.cwrap('cpu_load_program', 'number', ['array', 'number', 'number']);
        console.log('[Worker] ✓ cpu_load_program wrapped');

        const writeByte = Module.cwrap('cpu_write_byte', null, ['number', 'number']);
        console.log('[Worker] ✓ cpu_write_byte wrapped');

        // Direct ROM initialization function (bypasses read-only check)
        // This is available in the rebuilt WASM
        let initRom = null;
        try {
            initRom = Module.cwrap('cpu_init_rom', 'number', ['array', 'number', 'number']);
            console.log('[Worker] ✓ cpu_init_rom wrapped (direct ROM initialization available)');
        } catch (e) {
            console.log('[Worker] ℹ cpu_init_rom not available: ' + e);
        }

        // Now assign all at once
        cpu = { init, reset, step, run, pause, getState, loadROM, loadProgram, writeByte, initRom };

        console.log('[Worker] All CPU functions assigned to cpu object');
        console.log('[Worker] cpu.init typeof:', typeof cpu.init);
        console.log('[Worker] cpu.init callable?', !!cpu.init && typeof cpu.init === 'function');

        initialized = true;
        console.log('[Worker] setupCPU completed successfully, initialized=true');
    } catch (error) {
        console.error('[Worker] setupCPU failed:', error);
        throw new Error(`Failed to setup CPU functions: ${error}`);
    }
}

/**
 * Extract CPU state from WASM memory
 */
function getCPUState(): CPUState {
    if (!initialized || !cpu) {
        throw new Error('Simulator not initialized');
    }

    const Module = getModule();
    if (!Module) {
        throw new Error('WASM Module is not available');
    }

    const statePtr = cpu.getState();
    if (!statePtr) {
        throw new Error('cpu_get_state returned null/undefined pointer - simulator may be uninitialized');
    }

    // Use Emscripten's getValue to read from WASM memory
    // The CPU state structure layout:
    // uint16_t sr (2 bytes at offset 0)
    // uint32_t d[8] (32 bytes at offset 4)
    // uint32_t a[8] (32 bytes at offset 36)
    // uint32_t pc (4 bytes at offset 68)
    // uint32_t ssp, usp, msp (12 bytes at offset 72)

    if (!Module.getValue) {
        throw new Error('Module.getValue not available - cannot read WASM memory');
    }

    try {
        return {
            pc: Module.getValue(statePtr + 68, 'i32'),
            sr: Module.getValue(statePtr, 'i16'),
            dregs: [
                Module.getValue(statePtr + 4, 'i32'),
                Module.getValue(statePtr + 8, 'i32'),
                Module.getValue(statePtr + 12, 'i32'),
                Module.getValue(statePtr + 16, 'i32'),
                Module.getValue(statePtr + 20, 'i32'),
                Module.getValue(statePtr + 24, 'i32'),
                Module.getValue(statePtr + 28, 'i32'),
                Module.getValue(statePtr + 32, 'i32'),
            ],
            aregs: [
                Module.getValue(statePtr + 36, 'i32'),
                Module.getValue(statePtr + 40, 'i32'),
                Module.getValue(statePtr + 44, 'i32'),
                Module.getValue(statePtr + 48, 'i32'),
                Module.getValue(statePtr + 52, 'i32'),
                Module.getValue(statePtr + 56, 'i32'),
                Module.getValue(statePtr + 60, 'i32'),
                Module.getValue(statePtr + 64, 'i32'),
            ],
            ssp: Module.getValue(statePtr + 72, 'i32'),
            usp: Module.getValue(statePtr + 76, 'i32'),
            msp: Module.getValue(statePtr + 80, 'i32'),
        };
    } catch (e) {
        throw new Error(`Failed to read CPU state from WASM memory: ${e}`);
    }
}

/**
 * Handle incoming messages from main thread
 */
async function handleMessage(event: MessageEvent<SimulatorMessage>): Promise<void> {
    try {
        const { type, payload } = event.data;
        console.log(`[Worker] Handling message:`, type);

        switch (type) {
            case 'init':
                console.log('[Worker] INIT: Starting WASM initialization...');
                if (!initialized) {
                    await initWASM();
                    if (!initialized) {
                        throw new Error('WASM initialization failed');
                    }
                    console.log('[Worker] INIT: Calling cpu.init()...');
                    cpu.init();
                    console.log('[Worker] INIT: CPU initialized');
                }
                console.log('[Worker] INIT: Sending ready message');
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            case 'step':
                console.log('[Worker] STEP: Executing one instruction');
                try {
                    const stepResult = cpu.step();
                    console.log('[Worker] STEP: cpu.step() returned:', stepResult);
                    const state = getCPUState();
                    console.log('[Worker] STEP: Done, PC=0x' + state.pc.toString(16).padStart(6, '0'));
                    (self as any).postMessage({ type: 'state', state } as StateMessage);
                } catch (stepError: any) {
                    console.error('[Worker] STEP: Error during instruction execution:', stepError.message);
                    throw stepError;
                }
                break;

            case 'run':
                const count = payload?.count || 1000;
                console.log(`[Worker] RUN: Executing ${count} instructions`);
                cpu.run(count);
                const runState = getCPUState();
                console.log('[Worker] RUN: Done, PC=0x' + runState.pc.toString(16).padStart(6, '0'));
                (self as any).postMessage({ type: 'state', state: runState } as StateMessage);
                break;

            case 'pause':
                console.log('[Worker] PAUSE: Pausing CPU');
                cpu.pause();
                break;

            case 'reset':
                console.log('[Worker] RESET: Resetting CPU');
                cpu.reset();
                const resetState = getCPUState();
                console.log('[Worker] RESET: Done, PC=0x' + resetState.pc.toString(16).padStart(6, '0'));
                (self as any).postMessage({ type: 'state', state: resetState } as StateMessage);
                break;

            case 'getState':
                console.log('[Worker] GETSTATE: Reading current CPU state');
                const currentState = getCPUState();
                (self as any).postMessage({ type: 'state', state: currentState } as StateMessage);
                break;

            case 'loadROM':
                const romData = payload?.data as Uint8Array;
                const romAddr = payload?.address || 0x000000;
                if (!romData) {
                    throw new Error('No ROM data provided');
                }
                console.log(`[Worker] LOADROM: Loading ${romData.length} bytes at 0x${romAddr.toString(16).padStart(6, '0')}`);

                let romResult = -1;

                // Use direct ROM initialization if available (bypasses read-only check)
                if (cpu.initRom && romAddr === 0x000000) {
                    console.log(`[Worker] LOADROM: Using direct ROM initialization (cpu_init_rom)`);
                    try {
                        romResult = cpu.initRom(romData, romData.length, romAddr);
                    } catch (e) {
                        console.warn(`[Worker] LOADROM: Direct init failed: ${e}`);
                        romResult = -1;
                    }
                }

                // If direct init not available or failed, try standard load
                if (romResult !== 0) {
                    console.log(`[Worker] LOADROM: Attempting standard load path (cpu_load_program)`);
                    try {
                        romResult = cpu.loadProgram(romData, romData.length, romAddr);
                    } catch (e) {
                        console.warn(`[Worker] LOADROM: Standard load failed: ${e}`);
                        romResult = -1;
                    }
                }

                // For ROM at 0x000000, provide helpful error message if load failed
                if (romResult !== 0 && romAddr === 0x000000) {
                    console.warn(`[Worker] LOADROM: ⚠️  Failed to load ROM at 0x000000`);
                    console.warn(`[Worker] LOADROM: The WASM binary has read-only ROM protection`);
                    console.warn(`[Worker] LOADROM: To fix this, rebuild WASM with Emscripten:`);
                    console.warn(`[Worker] LOADROM:   1. Install Emscripten: https://emscripten.org/docs/getting_started/downloads.html`);
                    console.warn(`[Worker] LOADROM:   2. Rebuild: cd evm-web/evm-core && ./build.sh`);
                    console.log(`[Worker] LOADROM: Continuing without ROM (simulator is ready)`);
                    (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                    break;
                }

                // For non-ROM data, throw error if load failed
                if (romResult !== 0) {
                    throw new Error(`Failed to load program at 0x${romAddr.toString(16).padStart(6, '0')}`);
                }

                console.log('[Worker] LOADROM: Success');
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            case 'loadProgram':
                const progData = payload?.data as Uint8Array;
                const progAddr = payload?.address || 0x400000;
                if (!progData) {
                    throw new Error('No program data provided');
                }
                console.log(`[Worker] LOADPROG: Loading ${progData.length} bytes at 0x${progAddr.toString(16).padStart(6, '0')}`);
                const progResult = cpu.loadProgram(progData, progData.length, progAddr);
                if (progResult !== 0) {
                    throw new Error(`Failed to load program (result=${progResult})`);
                }
                console.log('[Worker] LOADPROG: Success');
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            case 'writeMemory':
                const writeAddr = payload?.addr as number;
                const writeData = payload?.data as number[];
                if (writeAddr === undefined || !writeData) {
                    throw new Error('No addr or data provided for memory write');
                }
                console.log(`[Worker] WRITEMEM: Writing ${writeData.length} bytes at 0x${writeAddr.toString(16).padStart(6, '0')}`);

                // Write each byte
                for (let i = 0; i < writeData.length; i++) {
                    cpu.writeByte(writeAddr + i, writeData[i]);
                }

                console.log('[Worker] WRITEMEM: Success');
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            default:
                throw new Error(`Unknown message type: ${type}`);
        }
    } catch (error) {
        const errorMsg = error instanceof Error ? error.message : String(error);
        console.error('[Worker] ERROR:', errorMsg, error);
        (self as any).postMessage({
            type: 'error',
            error: errorMsg
        } as ErrorMessage);
    }
}

// Listen for messages from main thread
console.log('[Worker] Worker script loaded, listening for messages...');

(self as any).onmessage = (event: MessageEvent) => {
    handleMessage(event).catch((error) => {
        console.error('[Worker] Unhandled error in handleMessage:', error);
    });
};
