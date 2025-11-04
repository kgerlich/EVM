import React from 'react';
import styles from './PeripheralMonitor.module.css';

interface RegisterValue {
    name: string;
    value: number;
    description: string;
    format?: 'hex' | 'binary' | 'decimal';
}

interface PeripheralMonitorProps {
    title?: string;
}

const PIT_68230_REGISTERS: RegisterValue[] = [
    { name: 'PGCR', value: 0, description: 'Port General Control', format: 'hex' },
    { name: 'PADDR', value: 0, description: 'Port A Data Direction', format: 'hex' },
    { name: 'PBDDR', value: 0, description: 'Port B Data Direction', format: 'hex' },
    { name: 'PCDR', value: 0, description: 'Port C Data Direction', format: 'hex' },
    { name: 'PACR', value: 0, description: 'Port A Control', format: 'hex' },
    { name: 'PBCR', value: 0, description: 'Port B Control', format: 'hex' },
    { name: 'PCCR', value: 0, description: 'Port C Control', format: 'hex' },
    { name: 'TIVR', value: 0, description: 'Timer Interrupt Vector', format: 'hex' },
    { name: 'TCR', value: 0, description: 'Timer Control', format: 'hex' },
    { name: 'TISR', value: 0, description: 'Timer Interrupt Status', format: 'hex' },
    { name: 'CPRH', value: 0, description: 'Counter Preload (High)', format: 'hex' },
    { name: 'CPRM', value: 0, description: 'Counter Preload (Mid)', format: 'hex' },
    { name: 'CPRL', value: 0, description: 'Counter Preload (Low)', format: 'hex' },
];

const UART_68681_REGISTERS: RegisterValue[] = [
    { name: 'SR', value: 0, description: 'Status Register', format: 'binary' },
    { name: 'RHR', value: 0, description: 'Receive Holding', format: 'hex' },
    { name: 'THR', value: 0, description: 'Transmit Holding', format: 'hex' },
    { name: 'IMR', value: 0, description: 'Interrupt Mask', format: 'hex' },
    { name: 'ISR', value: 0, description: 'Interrupt Status', format: 'hex' },
    { name: 'ACR', value: 0, description: 'Auxiliary Control', format: 'hex' },
    { name: 'BDR', value: 0, description: 'Baud Rate', format: 'decimal' },
    { name: 'MR1', value: 0, description: 'Mode Register 1', format: 'hex' },
    { name: 'MR2', value: 0, description: 'Mode Register 2', format: 'hex' },
];

export const PeripheralMonitor: React.FC<PeripheralMonitorProps> = ({ title = 'Peripheral Registers' }) => {
    const formatValue = (value: number, format?: string): string => {
        switch (format) {
            case 'hex':
                return '0x' + value.toString(16).toUpperCase().padStart(2, '0');
            case 'binary':
                return '0b' + value.toString(2).padStart(8, '0');
            case 'decimal':
                return value.toString();
            default:
                return '0x' + value.toString(16).toUpperCase().padStart(2, '0');
        }
    };

    return (
        <div className={styles.container}>
            <h2>{title}</h2>

            <div className={styles.peripheral}>
                <h3>MC68230 Parallel Interface/Timer (PIT)</h3>
                <div className={styles.registerGrid}>
                    {PIT_68230_REGISTERS.map((reg) => (
                        <div key={reg.name} className={styles.register}>
                            <div className={styles.registerName}>{reg.name}</div>
                            <div className={styles.registerValue}>{formatValue(reg.value, reg.format)}</div>
                            <div className={styles.registerDesc}>{reg.description}</div>
                        </div>
                    ))}
                </div>
            </div>

            <div className={styles.peripheral}>
                <h3>MC68681 Dual UART</h3>
                <div className={styles.registerGrid}>
                    {UART_68681_REGISTERS.map((reg) => (
                        <div key={reg.name} className={styles.register}>
                            <div className={styles.registerName}>{reg.name}</div>
                            <div className={styles.registerValue}>{formatValue(reg.value, reg.format)}</div>
                            <div className={styles.registerDesc}>{reg.description}</div>
                        </div>
                    ))}
                </div>
            </div>

            <div className={styles.note}>
                <strong>Note:</strong> Register values are placeholders. Connect to running simulator to see live updates.
            </div>
        </div>
    );
};
