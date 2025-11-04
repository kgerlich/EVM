import React, { useMemo } from 'react';
import { CPUState } from '../hooks/useSimulator';
import styles from './Disassembler.module.css';

interface DisassemblerProps {
    state: CPUState | null;
    memory: Uint8Array | null;
}

interface Instruction {
    address: number;
    opcode: string;
    mnemonic: string;
    bytes: string;
    cycles: number;
}

const INSTRUCTION_SET: { [key: number]: { mnem: string; cycles: number } } = {
    0x0000: { mnem: 'ORI', cycles: 8 },
    0x0200: { mnem: 'ANDI', cycles: 8 },
    0x0400: { mnem: 'SUBI', cycles: 8 },
    0x0600: { mnem: 'ADDI', cycles: 8 },
    0x0800: { mnem: 'BTST', cycles: 10 },
    0x0a00: { mnem: 'EORI', cycles: 8 },
    0x0c00: { mnem: 'CMPI', cycles: 8 },
    0x1000: { mnem: 'MOVE.B', cycles: 4 },
    0x3000: { mnem: 'MOVE.W', cycles: 4 },
    0x2000: { mnem: 'MOVE.L', cycles: 4 },
    0x4000: { mnem: 'CHK', cycles: 10 },
    0x4200: { mnem: 'CLR', cycles: 4 },
    0x4400: { mnem: 'NEG', cycles: 4 },
    0x4600: { mnem: 'NEGX', cycles: 4 },
    0x4800: { mnem: 'SWAP', cycles: 4 },
    0x4a00: { mnem: 'TST', cycles: 4 },
    0x4e40: { mnem: 'TRAP', cycles: 34 },
    0x4e50: { mnem: 'LINK', cycles: 16 },
    0x4e58: { mnem: 'UNLK', cycles: 12 },
    0x4e60: { mnem: 'MOVE USP', cycles: 4 },
    0x4e70: { mnem: 'RESET', cycles: 132 },
    0x4e71: { mnem: 'NOP', cycles: 4 },
    0x4e72: { mnem: 'STOP', cycles: 4 },
    0x4e73: { mnem: 'RTE', cycles: 20 },
    0x4e75: { mnem: 'RTS', cycles: 16 },
    0x4e76: { mnem: 'TRAPV', cycles: 34 },
    0x4e77: { mnem: 'RTR', cycles: 20 },
    0x5000: { mnem: 'ADDQ', cycles: 4 },
    0x5100: { mnem: 'SUBQ', cycles: 4 },
    0x6000: { mnem: 'BRA', cycles: 10 },
    0x6100: { mnem: 'BSR', cycles: 18 },
    0x7000: { mnem: 'MOVEQ', cycles: 4 },
    0x8000: { mnem: 'OR', cycles: 4 },
    0x9000: { mnem: 'SUB', cycles: 4 },
    0xb000: { mnem: 'CMP', cycles: 4 },
    0xc000: { mnem: 'AND', cycles: 4 },
    0xd000: { mnem: 'ADD', cycles: 4 },
    0xe000: { mnem: 'SHIFT', cycles: 6 },
};

export const Disassembler: React.FC<DisassemblerProps> = ({ state, memory }) => {
    const instructions = useMemo(() => {
        if (!state || !memory) return [];

        const result: Instruction[] = [];
        let addr = state.pc;
        const maxInstructions = 10;

        for (let i = 0; i < maxInstructions && addr < memory.length - 1; i++) {
            const byte1 = memory[addr];
            const byte2 = memory[addr + 1];
            const opcode = (byte1 << 8) | byte2;

            // Look up instruction
            const baseOpcode = opcode & 0xf000;
            const instInfo = INSTRUCTION_SET[baseOpcode] || { mnem: 'UNKNOWN', cycles: 0 };

            result.push({
                address: addr,
                opcode: '0x' + opcode.toString(16).toUpperCase().padStart(4, '0'),
                mnemonic: instInfo.mnem,
                bytes: byte1.toString(16).toUpperCase().padStart(2, '0') + ' ' +
                       byte2.toString(16).toUpperCase().padStart(2, '0'),
                cycles: instInfo.cycles,
            });

            addr += 2;
        }

        return result;
    }, [state, memory]);

    const formatHex = (value: number, chars: number = 6): string => {
        return '0x' + value.toString(16).toUpperCase().padStart(chars, '0');
    };

    return (
        <div className={styles.container}>
            <h2>Disassembler</h2>

            {state && (
                <div className={styles.currentPC}>
                    Current PC: <span className={styles.pcValue}>{formatHex(state.pc, 6)}</span>
                </div>
            )}

            {instructions.length === 0 ? (
                <div className={styles.empty}>No instructions available</div>
            ) : (
                <div className={styles.instructionList}>
                    <div className={styles.tableHeader}>
                        <div className={styles.col_address}>Address</div>
                        <div className={styles.col_bytes}>Bytes</div>
                        <div className={styles.col_mnemonic}>Instruction</div>
                        <div className={styles.col_cycles}>Cycles</div>
                    </div>

                    {instructions.map((inst, index) => (
                        <div
                            key={index}
                            className={`${styles.instruction} ${
                                state && inst.address === state.pc ? styles.current : ''
                            }`}
                        >
                            <div className={styles.col_address}>{formatHex(inst.address, 6)}</div>
                            <div className={styles.col_bytes}>{inst.bytes}</div>
                            <div className={styles.col_mnemonic}>{inst.mnemonic}</div>
                            <div className={styles.col_cycles}>{inst.cycles}</div>
                        </div>
                    ))}
                </div>
            )}
        </div>
    );
};
