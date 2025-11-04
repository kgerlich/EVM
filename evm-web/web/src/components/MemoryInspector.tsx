import React, { useState, useCallback } from 'react';
import { UseSimulatorReturn } from '../hooks/useSimulator';
import styles from './MemoryInspector.module.css';

interface MemoryInspectorProps {
    simulator: UseSimulatorReturn;
}

const BYTES_PER_ROW = 16;

export const MemoryInspector: React.FC<MemoryInspectorProps> = ({ simulator }) => {
    const [address, setAddress] = useState('0x400000');
    const [memoryData, setMemoryData] = useState<Uint8Array | null>(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState<string | null>(null);

    const parseAddress = (addr: string): number => {
        const trimmed = addr.trim();
        if (trimmed.startsWith('0x') || trimmed.startsWith('0X')) {
            return parseInt(trimmed, 16);
        }
        return parseInt(trimmed, 10);
    };

    const loadMemory = useCallback(async () => {
        try {
            setLoading(true);
            setError(null);
            const addr = parseAddress(address);
            const data = await simulator.readMemory(addr, 256);
            setMemoryData(data);
        } catch (err) {
            setError(err instanceof Error ? err.message : 'Failed to read memory');
            setMemoryData(null);
        } finally {
            setLoading(false);
        }
    }, [address, simulator]);

    const formatHex = (value: number, chars: number = 2): string => {
        return value.toString(16).toUpperCase().padStart(chars, '0');
    };

    const handleKeyPress = (e: React.KeyboardEvent) => {
        if (e.key === 'Enter') {
            loadMemory();
        }
    };

    const isROM = (): boolean => {
        try {
            const addr = parseAddress(address);
            return addr >= 0x000000 && addr < 0x010000;
        } catch {
            return false;
        }
    };

    return (
        <div className={styles.container}>
            <h2>Memory Inspector</h2>

            <div className={styles.controls}>
                <div className={styles.inputGroup}>
                    <label>Address (hex or decimal):</label>
                    <input
                        type="text"
                        value={address}
                        onChange={(e) => setAddress(e.target.value)}
                        onKeyPress={handleKeyPress}
                        placeholder="0x400000"
                        className={styles.input}
                    />
                    <button onClick={loadMemory} disabled={loading} className={styles.button}>
                        {loading ? 'Loading...' : 'Read'}
                    </button>
                </div>

                {isROM() && <span className={styles.romWarning}>ROM (read-only)</span>}
            </div>

            {error && <div className={styles.error}>{error}</div>}

            {memoryData && (
                <div className={styles.memoryView}>
                    <div className={styles.hexDump}>
                        {Array.from({ length: Math.ceil(memoryData.length / BYTES_PER_ROW) }).map(
                            (_, rowIndex) => {
                                const startOffset = rowIndex * BYTES_PER_ROW;
                                const endOffset = Math.min(startOffset + BYTES_PER_ROW, memoryData.length);
                                const baseAddr = parseAddress(address) + startOffset;

                                return (
                                    <div key={rowIndex} className={styles.hexRow}>
                                        <div className={styles.address}>
                                            {formatHex(baseAddr >> 16, 4)}:
                                            {formatHex(baseAddr & 0xFFFF, 4)}
                                        </div>

                                        <div className={styles.hexBytes}>
                                            {Array.from({ length: BYTES_PER_ROW }).map((_, byteIndex) => {
                                                const offset = startOffset + byteIndex;
                                                const byte = offset < memoryData.length ? memoryData[offset] : null;

                                                return (
                                                    <span
                                                        key={byteIndex}
                                                        className={`${styles.byte} ${
                                                            byte !== null ? styles.hasByte : styles.empty
                                                        }`}
                                                    >
                                                        {byte !== null ? formatHex(byte) : '--'}
                                                    </span>
                                                );
                                            })}
                                        </div>

                                        <div className={styles.ascii}>
                                            {Array.from({
                                                length: Math.min(BYTES_PER_ROW, endOffset - startOffset),
                                            }).map((_, byteIndex) => {
                                                const offset = startOffset + byteIndex;
                                                const byte = memoryData[offset];
                                                const char =
                                                    byte >= 32 && byte < 127 ? String.fromCharCode(byte) : '.';
                                                return (
                                                    <span key={byteIndex} className={styles.asciiChar}>
                                                        {char}
                                                    </span>
                                                );
                                            })}
                                        </div>
                                    </div>
                                );
                            }
                        )}
                    </div>
                </div>
            )}
        </div>
    );
};
