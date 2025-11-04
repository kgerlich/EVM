import React, { useState, useEffect } from 'react';
import { useSimulator } from './hooks/useSimulator';
import { CPURegisters } from './components/CPURegisters';
import { MemoryInspector } from './components/MemoryInspector';
import { Terminal } from './components/Terminal';
import { Disassembler } from './components/Disassembler';
import { PeripheralMonitor } from './components/PeripheralMonitor';
import { ControlPanel } from './components/ControlPanel';
import './App.css';

function App() {
    const simulator = useSimulator();
    const [memory, setMemory] = useState<Uint8Array | null>(null);

    // Periodically update memory for disassembler
    useEffect(() => {
        if (simulator.state && simulator.initialized) {
            const fetchMemory = async () => {
                try {
                    const data = await simulator.readMemory(simulator.state!.pc, 512);
                    setMemory(data);
                } catch (error) {
                    console.error('Failed to read memory:', error);
                }
            };
            fetchMemory();
        }
    }, [simulator.state, simulator.initialized]);

    return (
        <div className="app">
            <header className="app-header">
                <h1>🖥️ EVM MC68020 Simulator</h1>
                <p>WebAssembly-based 68K CPU emulator running in your browser</p>
            </header>

            <div className="app-container">
                {/* Left Column: Control Panel + Registers + Disassembler */}
                <div className="column left-column">
                    <div className="panel">
                        <ControlPanel simulator={simulator} />
                    </div>

                    <div className="panel">
                        <CPURegisters state={simulator.state} />
                    </div>

                    <div className="panel">
                        <Disassembler state={simulator.state} memory={memory} />
                    </div>
                </div>

                {/* Center Column: Memory Inspector + Peripherals */}
                <div className="column center-column">
                    <div className="panel">
                        <MemoryInspector simulator={simulator} />
                    </div>

                    <div className="panel">
                        <PeripheralMonitor />
                    </div>
                </div>

                {/* Right Column: Terminal */}
                <div className="column right-column">
                    <div className="panel">
                        <Terminal />
                    </div>
                </div>
            </div>

            <footer className="app-footer">
                <p>
                    MC68020 CPU Emulator •
                    <a href="https://github.com/kgerlich/EVM" target="_blank" rel="noopener noreferrer">
                        GitHub
                    </a>
                </p>
            </footer>
        </div>
    );
}

export default App;
