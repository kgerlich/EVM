import { useState, useEffect, useCallback, useRef } from 'react';
import { parseS19, mergeSegments } from '../utils/s19Parser';

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
        console.log('🚀 [useSimulator] Creating Web Worker...');

        // Create worker - need to use a compiled JS file, not TypeScript
        // Use a classic worker (not module) so it can use importScripts()
        try {
            workerRef.current = new Worker(
                new URL('../workers/simulator.worker.ts', import.meta.url),
                { type: 'classic' }
            );
            console.log('✅ [useSimulator] Worker created successfully');
        } catch (err) {
            console.error('❌ [useSimulator] Failed to create worker:', err);
            setSimulatorState((prev) => ({
                ...prev,
                error: `Failed to create worker: ${err}`,
            }));
            return;
        }

        const worker = workerRef.current;

        // Listen for messages from worker
        worker.onmessage = (event: MessageEvent) => {
            const { type, state, error, data, addr } = event.data;
            console.log(`📨 [useSimulator] Received message from worker:`, { type, state, error });

            switch (type) {
                case 'ready':
                    console.log('✅ [useSimulator] Simulator initialized and ready');
                    setSimulatorState((prev) => ({
                        ...prev,
                        initialized: true,
                        error: null,
                    }));
                    break;

                case 'state':
                    console.log('📊 [useSimulator] CPU State updated:', {
                        pc: `0x${state.pc.toString(16).padStart(6, '0')}`,
                        sr: `0x${state.sr.toString(16).padStart(4, '0')}`,
                        d0: `0x${state.dregs[0].toString(16).padStart(8, '0')}`,
                        a7: `0x${state.aregs[7].toString(16).padStart(8, '0')}`,
                    });
                    setSimulatorState((prev) => ({
                        ...prev,
                        state,
                        running: false,
                        error: null,
                    }));
                    break;

                case 'memoryRead':
                    console.log(`🔍 [useSimulator] Memory read from 0x${addr.toString(16).padStart(6, '0')}`);
                    const readCallback = memoryPromiseRef.current.get(`read-${addr}`);
                    if (readCallback) {
                        readCallback(new Uint8Array(data));
                        memoryPromiseRef.current.delete(`read-${addr}`);
                    }
                    break;

                case 'error':
                    console.error('❌ [useSimulator] Worker error:', error);
                    setSimulatorState((prev) => ({
                        ...prev,
                        error: error || 'Unknown error',
                        running: false,
                    }));
                    break;

                default:
                    console.warn(`⚠️ [useSimulator] Unknown message type: ${type}`);
            }
        };

        worker.onerror = (error) => {
            console.error('❌ [useSimulator] Worker error event:', error);
            setSimulatorState((prev) => ({
                ...prev,
                error: error.message || 'Worker error',
                running: false,
            }));
        };

        // Initialize simulator
        console.log('🔧 [useSimulator] Sending init message to worker...');
        worker.postMessage({ type: 'init' });

        // Cleanup
        return () => {
            if (workerRef.current) {
                console.log('🛑 [useSimulator] Terminating worker');
                workerRef.current.terminate();
            }
        };
    }, []);

    // Load ROM after initialization, then call reset to read reset vectors from ROM
    useEffect(() => {
        if (simulatorState.initialized && workerRef.current) {
            (async () => {
                try {
                    console.log('📥 [useSimulator] Loading ROM file...');
                    // Fetch S19 ROM file
                    const response = await fetch('/PS20.S19');
                    console.log(`✅ [useSimulator] Fetched PS20.S19, status: ${response.status}`);
                    const content = await response.text();
                    console.log(`✅ [useSimulator] ROM content length: ${content.length} bytes`);

                    // Parse S19 format
                    console.log('🔍 [useSimulator] Parsing S19 format...');
                    const { segments } = parseS19(content);
                    console.log(`✅ [useSimulator] Parsed ${segments.size} memory segments`);

                    // Load ROM into simulator - load each segment directly without merging
                    // (merging was destroying data, so we write each segment's bytes directly to WASM memory)
                    let totalBytes = 0;
                    for (const [addr, data] of segments) {
                        const romData = new Uint8Array(data);
                        totalBytes += romData.length;
                        console.log(`📍 [useSimulator] Loading segment at 0x${addr.toString(16).padStart(6, '0')}, size: ${romData.length} bytes`);

                        await new Promise<void>((resolve, reject) => {
                            const onMessage = (event: MessageEvent) => {
                                if (event.data.type === 'ready' || event.data.type === 'error') {
                                    workerRef.current?.removeEventListener('message', onMessage);
                                    if (event.data.type === 'error') {
                                        reject(new Error(event.data.error));
                                    } else {
                                        resolve();
                                    }
                                }
                            };

                            workerRef.current?.addEventListener('message', onMessage);
                            // Pass Uint8Array directly, not Array.from()
                            // Emscripten cwrap needs a typed array for proper pointer conversion
                            workerRef.current?.postMessage({
                                type: 'loadROM',
                                payload: { data: romData, address: addr },
                            });
                        });
                    }
                    console.log(`✅ [useSimulator] ROM loaded successfully! Total: ${totalBytes} bytes`);

                    // NOW that ROM is loaded, call reset to read reset vectors from ROM
                    console.log('🔄 [useSimulator] Calling reset to read reset vectors from loaded ROM...');
                    await new Promise<void>((resolve, reject) => {
                        const onMessage = (event: MessageEvent) => {
                            if (event.data.type === 'state' || event.data.type === 'error') {
                                workerRef.current?.removeEventListener('message', onMessage);
                                if (event.data.type === 'error') {
                                    reject(new Error(event.data.error));
                                } else {
                                    console.log(`✅ [useSimulator] Reset complete, PC=0x${event.data.state.pc.toString(16).padStart(6, '0')}, SSP=0x${event.data.state.ssp.toString(16).padStart(6, '0')}`);
                                    resolve();
                                }
                            }
                        };

                        workerRef.current?.addEventListener('message', onMessage);
                        workerRef.current?.postMessage({ type: 'reset' });
                    });

                } catch (error) {
                    const msg = error instanceof Error ? error.message : String(error);
                    console.error(`❌ [useSimulator] Failed to load ROM: ${msg}`);
                    setSimulatorState((prev) => ({
                        ...prev,
                        error: `Failed to load ROM: ${msg}`,
                    }));
                }
            })();
        }
    }, [simulatorState.initialized]);

    const step = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            console.log('⏭️ [useSimulator] Stepping one instruction');
            workerRef.current.postMessage({ type: 'step' });
        }
    }, [simulatorState.initialized]);

    const run = useCallback((count = 10000) => {
        if (workerRef.current && simulatorState.initialized) {
            console.log(`▶️ [useSimulator] Running ${count} instructions`);
            setSimulatorState((prev) => ({ ...prev, running: true }));
            workerRef.current.postMessage({ type: 'run', payload: { count } });
        }
    }, [simulatorState.initialized]);

    const pause = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            console.log('⏸️ [useSimulator] Pausing execution');
            setSimulatorState((prev) => ({ ...prev, paused: true, running: false }));
            workerRef.current.postMessage({ type: 'pause' });
        }
    }, [simulatorState.initialized]);

    const reset = useCallback(() => {
        if (workerRef.current && simulatorState.initialized) {
            console.log('🔄 [useSimulator] Resetting CPU');
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
