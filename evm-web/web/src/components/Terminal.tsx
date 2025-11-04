import React, { useState, useRef, useEffect } from 'react';
import styles from './Terminal.module.css';

interface TerminalProps {
    title?: string;
}

export const Terminal: React.FC<TerminalProps> = ({ title = 'Serial Terminal (68681 UART)' }) => {
    const [output, setOutput] = useState<string[]>([
        'EVM MC68020 Simulator - Serial Terminal',
        'Waiting for program output...',
        '',
    ]);
    const [input, setInput] = useState('');
    const outputRef = useRef<HTMLDivElement>(null);
    const inputRef = useRef<HTMLInputElement>(null);

    // Auto-scroll to bottom
    useEffect(() => {
        if (outputRef.current) {
            outputRef.current.scrollTop = outputRef.current.scrollHeight;
        }
    }, [output]);

    const addOutput = (text: string) => {
        setOutput((prev) => [...prev, text]);
    };

    const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        setInput(e.target.value);
    };

    const handleSendInput = () => {
        if (input.trim()) {
            addOutput(`> ${input}`);
            // TODO: Send input to simulator's 68681 UART
            setInput('');
            if (inputRef.current) {
                inputRef.current.focus();
            }
        }
    };

    const handleKeyPress = (e: React.KeyboardEvent) => {
        if (e.key === 'Enter') {
            handleSendInput();
        }
    };

    const clearTerminal = () => {
        setOutput(['Terminal cleared.', '']);
    };

    return (
        <div className={styles.container}>
            <div className={styles.header}>
                <h2>{title}</h2>
                <button onClick={clearTerminal} className={styles.clearButton}>
                    Clear
                </button>
            </div>

            <div className={styles.output} ref={outputRef}>
                {output.map((line, index) => (
                    <div key={index} className={styles.line}>
                        {line}
                    </div>
                ))}
            </div>

            <div className={styles.input}>
                <span className={styles.prompt}>$ </span>
                <input
                    ref={inputRef}
                    type="text"
                    value={input}
                    onChange={handleInputChange}
                    onKeyPress={handleKeyPress}
                    placeholder="Enter command..."
                    className={styles.inputField}
                    autoFocus
                />
                <button onClick={handleSendInput} className={styles.sendButton}>
                    Send
                </button>
            </div>
        </div>
    );
};
