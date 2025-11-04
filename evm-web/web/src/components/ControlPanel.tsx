import React, { useState, useRef } from 'react';
import { UseSimulatorReturn } from '../hooks/useSimulator';
import styles from './ControlPanel.module.css';

interface ControlPanelProps {
    simulator: UseSimulatorReturn;
}

export const ControlPanel: React.FC<ControlPanelProps> = ({ simulator }) => {
    const [speed, setSpeed] = useState(1000);
    const [isRunning, setIsRunning] = useState(false);
    const runIntervalRef = useRef<NodeJS.Timeout | null>(null);
    const romFileRef = useRef<HTMLInputElement>(null);
    const programFileRef = useRef<HTMLInputElement>(null);

    const handlePlay = () => {
        if (!isRunning) {
            setIsRunning(true);
            runIntervalRef.current = setInterval(() => {
                simulator.run(speed);
            }, 100); // Update every 100ms
        }
    };

    const handlePause = () => {
        if (isRunning) {
            setIsRunning(false);
            if (runIntervalRef.current) {
                clearInterval(runIntervalRef.current);
                runIntervalRef.current = null;
            }
            simulator.pause();
        }
    };

    const handleStep = () => {
        simulator.step();
    };

    const handleReset = () => {
        if (isRunning) {
            handlePause();
        }
        simulator.reset();
    };

    const handleLoadROM = async (event: React.ChangeEvent<HTMLInputElement>) => {
        const file = event.target.files?.[0];
        if (file) {
            try {
                const arrayBuffer = await file.arrayBuffer();
                const data = new Uint8Array(arrayBuffer);
                await simulator.loadROM(data);
            } catch (error) {
                console.error('Failed to load ROM:', error);
            }
        }
    };

    const handleLoadProgram = async (event: React.ChangeEvent<HTMLInputElement>) => {
        const file = event.target.files?.[0];
        if (file) {
            try {
                const arrayBuffer = await file.arrayBuffer();
                const data = new Uint8Array(arrayBuffer);
                await simulator.loadProgram(data, 0x400000);
            } catch (error) {
                console.error('Failed to load program:', error);
            }
        }
    };

    return (
        <div className={styles.container}>
            <div className={styles.section}>
                <h3>Execution Controls</h3>
                <div className={styles.buttonGroup}>
                    <button
                        onClick={handlePlay}
                        disabled={isRunning || !simulator.initialized}
                        className={`${styles.button} ${styles.play}`}
                        title="Run simulator"
                    >
                        ▶ Play
                    </button>
                    <button
                        onClick={handlePause}
                        disabled={!isRunning || !simulator.initialized}
                        className={`${styles.button} ${styles.pause}`}
                        title="Pause simulator"
                    >
                        ⏸ Pause
                    </button>
                    <button
                        onClick={handleStep}
                        disabled={isRunning || !simulator.initialized}
                        className={`${styles.button} ${styles.step}`}
                        title="Execute one instruction"
                    >
                        ⏭ Step
                    </button>
                    <button
                        onClick={handleReset}
                        disabled={!simulator.initialized}
                        className={`${styles.button} ${styles.reset}`}
                        title="Reset simulator"
                    >
                        🔄 Reset
                    </button>
                </div>
            </div>

            <div className={styles.section}>
                <h3>Execution Speed</h3>
                <div className={styles.speedControl}>
                    <input
                        type="range"
                        min="100"
                        max="10000"
                        step="100"
                        value={speed}
                        onChange={(e) => setSpeed(Number(e.target.value))}
                        className={styles.slider}
                    />
                    <div className={styles.speedLabel}>
                        {speed} instructions/update
                    </div>
                </div>
            </div>

            <div className={styles.section}>
                <h3>File Loading</h3>
                <div className={styles.fileGroup}>
                    <button
                        onClick={() => romFileRef.current?.click()}
                        className={`${styles.button} ${styles.file}`}
                        disabled={!simulator.initialized}
                    >
                        📂 Load ROM
                    </button>
                    <input
                        ref={romFileRef}
                        type="file"
                        onChange={handleLoadROM}
                        style={{ display: 'none' }}
                        accept=".bin,.hex,.s19"
                    />

                    <button
                        onClick={() => programFileRef.current?.click()}
                        className={`${styles.button} ${styles.file}`}
                        disabled={!simulator.initialized}
                    >
                        📂 Load Program
                    </button>
                    <input
                        ref={programFileRef}
                        type="file"
                        onChange={handleLoadProgram}
                        style={{ display: 'none' }}
                        accept=".bin,.hex,.s19"
                    />
                </div>
            </div>

            <div className={styles.section}>
                <h3>Status</h3>
                <div className={styles.status}>
                    <div>
                        <span className={styles.statusLabel}>Initialized:</span>
                        <span className={simulator.initialized ? styles.statusOK : styles.statusError}>
                            {simulator.initialized ? '✓' : '✗'}
                        </span>
                    </div>
                    <div>
                        <span className={styles.statusLabel}>Running:</span>
                        <span className={isRunning ? styles.statusRunning : styles.statusStopped}>
                            {isRunning ? '▶' : '⏸'}
                        </span>
                    </div>
                    {simulator.error && (
                        <div className={styles.error}>
                            <span>Error:</span>
                            <span>{simulator.error}</span>
                        </div>
                    )}
                </div>
            </div>
        </div>
    );
};
