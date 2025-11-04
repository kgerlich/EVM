import { useState, useEffect, useCallback, useRef } from 'react';

export interface CPUState {
    pc: number;
    sr: number;
    dregs: number[];
    aregs: number[];
    ssp: number;
    usp: number;
    msp: number;
}

export interface SimulatorState {
    initialized: boolean;
    running: boolean;
    paused: boolean;
    state: CPUState | null;
    error: string | null;
}

export interface UseSimulatorReturn extends SimulatorState {
    step: () => void;
    run: (count?: number) => void;
    pause: () => void;
    reset: () => void;
    readMemory: (addr: number, size?: number) => Promise<Uint8Array>;
    writeMemory: (addr: number, data: Uint8Array) => Promise<void>;
    loadROM: (data: Uint8Array) => Promise<void>;
    loadProgram: (data: Uint8Array, addr?: number) => Promise<void>;
    getState: () => void;
}

export function useSimulator(): UseSimulatorReturn {
    const [simulatorState, setSimulatorState] = useState<SimulatorState>({
        initialized: false,
        running: false,
        paused: false,
        state: null,
        error: null,
    });

    const workerRef = useRef<Worker | null>(null);
    const memoryPromiseRef = useRef<Map<string, Function>>(new Map());

    // Initialize worker and simulator
    useEffect(() => {
        // Create worker from TypeScript file
        const workerUrl = new URL('../workers/simulator.worker.ts', import.meta.url);
        workerRef.current = new Worker(workerUrl, { type: 'module' });

        const worker = workerRef.current;

        // Listen for messages from worker
        worker.onmessage = (event: MessageEvent) => {
            const { type, state, error, data, addr } = event.data;

            switch (type) {
                case 'ready':
                    setSimulatorState((prev) => ({
                        ...prev,
                        initialized: true,
                        error: null,
                    }));
                    break;

                case 'state':
                    setSimulatorState((prev) => ({
                        ...prev,
                        state,
                        running: false,
                        error: null,
                    }));
                    break;

                case 'memoryRead':
                    const readCallback = memoryPromiseRef.current.get(`read-${addr}`);
                    if (readCallback) {
                        readCallback(new Uint8Array(data));
                        memoryPromiseRef.current.delete(`read-${addr}`);
                    }
                    break;

                case 'error':
                    setSimulatorState((prev) => ({
                        ...prev,
                        error: error || 'Unknown error',
                        running: false,
                    }));
                    break;

                default:
                    console.warn(`Unknown message type: ${type}`);
            }
        };

        worker.onerror = (error) => {
            setSimulatorState((prev) => ({
                ...prev,
                error: error.message || 'Worker error',
                running: false,
            }));
        };

        // Initialize simulator
        worker.postMessage({ type: 'init' });

        // Cleanup
        return () => {
            if (workerRef.current) {
                workerRef.current.terminate();
            }
        };
    }, []);

    const step = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            workerRef.current.postMessage({ type: 'step' });
        }
    }, [simulatorState.initialized]);

    const run = useCallback((count = 10000) => {
        if (workerRef.current && simulatorState.initialized) {
            setSimulatorState((prev) => ({ ...prev, running: true }));
            workerRef.current.postMessage({ type: 'run', payload: { count } });
        }
    }, [simulatorState.initialized]);

    const pause = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            setSimulatorState((prev) => ({ ...prev, paused: true, running: false }));
            workerRef.current.postMessage({ type: 'pause' });
        }
    }, [simulatorState.initialized]);

    const reset = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            setSimulatorState((prev) => ({ ...prev, running: false, paused: false }));
            workerRef.current.postMessage({ type: 'reset' });
        }
    }, [simulatorState.initialized]);

    const readMemory = useCallback((addr: number, size = 256): Promise<Uint8Array> => {
        return new Promise((resolve, reject) => {
            if (!workerRef.current || !simulatorState.initialized) {
                reject(new Error('Simulator not initialized'));
                return;
            }

            const key = `read-${addr}`;
            memoryPromiseRef.current.set(key, resolve);

            setTimeout(() => {
                if (memoryPromiseRef.current.has(key)) {
                    memoryPromiseRef.current.delete(key);
                    reject(new Error('Memory read timeout'));
                }
            }, 5000);

            workerRef.current.postMessage({
                type: 'readMemory',
                payload: { addr, size },
            });
        });
    }, [simulatorState.initialized]);

    const writeMemory = useCallback((addr: number, data: Uint8Array): Promise<void> => {
        return new Promise((resolve, reject) => {
            if (!workerRef.current || !simulatorState.initialized) {
                reject(new Error('Simulator not initialized'));
                return;
            }

            workerRef.current.postMessage({
                type: 'writeMemory',
                payload: { addr, data: Array.from(data) },
            });

            resolve();
        });
    }, [simulatorState.initialized]);

    const loadROM = useCallback((data: Uint8Array): Promise<void> => {
        return new Promise((resolve, reject) => {
            if (!workerRef.current || !simulatorState.initialized) {
                reject(new Error('Simulator not initialized'));
                return;
            }

            const onMessage = (event: MessageEvent) => {
                if (event.data.type === 'ready') {
                    workerRef.current?.removeEventListener('message', onMessage);
                    resolve();
                }
            };

            workerRef.current.addEventListener('message', onMessage);

            workerRef.current.postMessage({
                type: 'loadROM',
                payload: { data: Array.from(data) },
            });
        });
    }, [simulatorState.initialized]);

    const loadProgram = useCallback((data: Uint8Array, addr = 0x400000): Promise<void> => {
        return new Promise((resolve, reject) => {
            if (!workerRef.current || !simulatorState.initialized) {
                reject(new Error('Simulator not initialized'));
                return;
            }

            const onMessage = (event: MessageEvent) => {
                if (event.data.type === 'ready') {
                    workerRef.current?.removeEventListener('message', onMessage);
                    resolve();
                }
            };

            workerRef.current.addEventListener('message', onMessage);

            workerRef.current.postMessage({
                type: 'loadProgram',
                payload: { data: Array.from(data), addr },
            });
        });
    }, [simulatorState.initialized]);

    const getState = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            workerRef.current.postMessage({ type: 'getState' });
        }
    }, [simulatorState.initialized]);

    return {
        ...simulatorState,
        step,
        run,
        pause,
        reset,
        readMemory,
        writeMemory,
        loadROM,
        loadProgram,
        getState,
    };
}
