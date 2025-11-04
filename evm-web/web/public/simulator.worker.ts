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

let Module: any;
let cpu: any;
let initialized = false;

/**
 * Initialize WASM module
 */
async function initWASM(): Promise<void> {
    return new Promise((resolve, reject) => {
        // WASM module initialization
        const script = document.createElement('script');
        script.src = '/evm.js';
        script.onload = () => {
            // Access the global Module created by evm.js
            if (typeof (window as any).Module !== 'undefined') {
                Module = (window as any).Module;
                Module.onRuntimeInitialized = () => {
                    cpu = {
                        init: Module.cwrap('cpu_init', 'number', []),
                        reset: Module.cwrap('cpu_reset', null, []),
                        step: Module.cwrap('cpu_step', null, []),
                        run: Module.cwrap('cpu_run', null, ['number']),
                        pause: Module.cwrap('cpu_pause', null, []),
                        getState: Module.cwrap('cpu_get_state', 'number', []),
                        readByte: Module.cwrap('cpu_read_byte', 'number', ['number']),
                        readWord: Module.cwrap('cpu_read_word', 'number', ['number']),
                        readDword: Module.cwrap('cpu_read_dword', 'number', ['number']),
                        readMemoryRange: Module.cwrap('cpu_read_memory_range', null, ['number', 'number', 'number']),
                        writeByte: Module.cwrap('cpu_write_byte', null, ['number', 'number']),
                        writeWord: Module.cwrap('cpu_write_word', null, ['number', 'number']),
                        writeDword: Module.cwrap('cpu_write_dword', null, ['number', 'number']),
                        writeMemoryRange: Module.cwrap('cpu_write_memory_range', null, ['number', 'number', 'number']),
                        loadROM: Module.cwrap('cpu_load_rom', null, ['number', 'number']),
                        loadProgram: Module.cwrap('cpu_load_program', null, ['number', 'number', 'number']),
                    };
                    initialized = true;
                    resolve();
                };
            } else {
                reject(new Error('WASM module failed to load'));
            }
        };
        script.onerror = () => {
            reject(new Error('Failed to load evm.js'));
        };
        document.head.appendChild(script);
    });
}

/**
 * Extract CPU state from WASM memory
 */
function getCPUState(): CPUState {
    if (!initialized || !cpu) {
        throw new Error('Simulator not initialized');
    }

    const statePtr = cpu.getState();
    const memView = new Uint32Array(Module.HEAP32.buffer, statePtr, 11);

    return {
        pc: memView[0],
        sr: memView[1],
        dregs: [memView[2], memView[3], memView[4], memView[5], memView[6], memView[7], memView[8], memView[9]],
        aregs: [memView[10], memView[11], memView[12], memView[13], memView[14], memView[15], memView[16], memView[17]],
        ssp: memView[18],
        usp: memView[19],
        msp: memView[20],
    };
}

/**
 * Handle incoming messages from main thread
 */
async function handleMessage(event: MessageEvent<SimulatorMessage>): Promise<void> {
    try {
        const { type, payload } = event.data;

        switch (type) {
            case 'init':
                if (!initialized) {
                    await initWASM();
                    cpu.init();
                }
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            case 'step':
                cpu.step();
                const state = getCPUState();
                (self as any).postMessage({ type: 'state', state } as StateMessage);
                break;

            case 'run':
                const count = payload?.count || 1000;
                cpu.run(count);
                const runState = getCPUState();
                (self as any).postMessage({ type: 'state', state: runState } as StateMessage);
                break;

            case 'pause':
                cpu.pause();
                break;

            case 'reset':
                cpu.reset();
                const resetState = getCPUState();
                (self as any).postMessage({ type: 'state', state: resetState } as StateMessage);
                break;

            case 'getState':
                const currentState = getCPUState();
                (self as any).postMessage({ type: 'state', state: currentState } as StateMessage);
                break;

            case 'readMemory':
                const { addr, size } = payload;
                const bufSize = size || 256;
                const bufPtr = Module._malloc(bufSize);
                cpu.readMemoryRange(bufPtr, addr, bufSize);
                const data = new Uint8Array(Module.HEAP8.buffer, bufPtr, bufSize);
                (self as any).postMessage({
                    type: 'memoryRead',
                    addr,
                    data: Array.from(data)
                });
                Module._free(bufPtr);
                break;

            case 'writeMemory':
                const { addr: writeAddr, data: writeData } = payload;
                const writeBufPtr = Module._malloc(writeData.length);
                const writeBuffer = new Uint8Array(Module.HEAP8.buffer, writeBufPtr, writeData.length);
                writeBuffer.set(writeData);
                cpu.writeMemoryRange(writeBufPtr, writeAddr, writeData.length);
                Module._free(writeBufPtr);
                break;

            case 'loadROM':
                const { data: romData } = payload;
                const romBufPtr = Module._malloc(romData.length);
                const romBuffer = new Uint8Array(Module.HEAP8.buffer, romBufPtr, romData.length);
                romBuffer.set(romData);
                cpu.loadROM(romBufPtr, romData.length);
                Module._free(romBufPtr);
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            case 'loadProgram':
                const { data: progData, addr: progAddr } = payload;
                const progBufPtr = Module._malloc(progData.length);
                const progBuffer = new Uint8Array(Module.HEAP8.buffer, progBufPtr, progData.length);
                progBuffer.set(progData);
                cpu.loadProgram(progBufPtr, progData.length, progAddr);
                Module._free(progBufPtr);
                (self as any).postMessage({ type: 'ready' } as ReadyMessage);
                break;

            default:
                throw new Error(`Unknown message type: ${type}`);
        }
    } catch (error) {
        const errorMsg = error instanceof Error ? error.message : String(error);
        (self as any).postMessage({
            type: 'error',
            error: errorMsg
        } as ErrorMessage);
    }
}

// Listen for messages from main thread
(self as any).onmessage = (event: MessageEvent) => {
    handleMessage(event).catch((error) => {
        console.error('Worker error:', error);
    });
};
