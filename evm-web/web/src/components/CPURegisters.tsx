import React from 'react';
import { CPUState } from '../hooks/useSimulator';
import styles from './CPURegisters.module.css';

interface CPURegistersProps {
    state: CPUState | null;
}

const FLAG_NAMES: { [key: number]: string } = {
    0x8000: 'T',  // Trace
    0x4000: 'S',  // Supervisor
    0x2000: 'M',  // Master
    0x1000: 'I2', // Interrupt priority
    0x0800: 'I1',
    0x0400: 'I0',
    0x0010: 'X',  // Extend
    0x0008: 'N',  // Negative
    0x0004: 'Z',  // Zero
    0x0002: 'V',  // Overflow
    0x0001: 'C',  // Carry
};

export const CPURegisters: React.FC<CPURegistersProps> = ({ state }) => {
    const formatHex = (value: number, bits: number = 32): string => {
        const hex = value.toString(16).toUpperCase();
        const chars = bits / 4;
        return '0x' + hex.padStart(chars, '0');
    };

    const getFlags = (): string[] => {
        if (!state) return [];
        const flags: string[] = [];
        const sr = state.sr & 0xFFFF;

        if (sr & 0x8000) flags.push('T');
        if (sr & 0x4000) flags.push('S');
        if (sr & 0x2000) flags.push('M');

        // Interrupt priority level
        const ipl = (sr >> 8) & 0x7;
        if (ipl > 0) flags.push(`IPL${ipl}`);

        if (sr & 0x0010) flags.push('X');
        if (sr & 0x0008) flags.push('N');
        if (sr & 0x0004) flags.push('Z');
        if (sr & 0x0002) flags.push('V');
        if (sr & 0x0001) flags.push('C');

        return flags;
    };

    if (!state) {
        return <div className={styles.container}>Loading...</div>;
    }

    return (
        <div className={styles.container}>
            <h2>CPU Registers</h2>

            {/* Program Counter */}
            <div className={styles.section}>
                <h3>Program Counter</h3>
                <div className={styles.register}>
                    <span className={styles.label}>PC:</span>
                    <span className={styles.value}>{formatHex(state.pc, 24)}</span>
                </div>
            </div>

            {/* Status Register */}
            <div className={styles.section}>
                <h3>Status Register</h3>
                <div className={styles.register}>
                    <span className={styles.label}>SR:</span>
                    <span className={styles.value}>{formatHex(state.sr, 16)}</span>
                </div>
                <div className={styles.flags}>
                    {getFlags().map((flag) => (
                        <span key={flag} className={styles.flag}>
                            {flag}
                        </span>
                    ))}
                </div>
            </div>

            {/* Data Registers */}
            <div className={styles.section}>
                <h3>Data Registers</h3>
                <div className={styles.registerGrid}>
                    {state.dregs.map((value, index) => (
                        <div key={`d${index}`} className={styles.register}>
                            <span className={styles.label}>D{index}:</span>
                            <span className={styles.value}>{formatHex(value, 32)}</span>
                        </div>
                    ))}
                </div>
            </div>

            {/* Address Registers */}
            <div className={styles.section}>
                <h3>Address Registers</h3>
                <div className={styles.registerGrid}>
                    {state.aregs.map((value, index) => (
                        <div key={`a${index}`} className={styles.register}>
                            <span className={styles.label}>A{index}:</span>
                            <span className={styles.value}>{formatHex(value, 32)}</span>
                        </div>
                    ))}
                </div>
            </div>

            {/* Stack Pointers */}
            <div className={styles.section}>
                <h3>Stack Pointers</h3>
                <div className={styles.registerGrid}>
                    <div className={styles.register}>
                        <span className={styles.label}>SSP:</span>
                        <span className={styles.value}>{formatHex(state.ssp, 32)}</span>
                    </div>
                    <div className={styles.register}>
                        <span className={styles.label}>USP:</span>
                        <span className={styles.value}>{formatHex(state.usp, 32)}</span>
                    </div>
                    <div className={styles.register}>
                        <span className={styles.label}>MSP:</span>
                        <span className={styles.value}>{formatHex(state.msp, 32)}</span>
                    </div>
                </div>
            </div>
        </div>
    );
};
