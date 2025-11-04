#!/usr/bin/env node

import puppeteer from 'puppeteer';
import http from 'http';

const PORT = 8084;
const BASE_URL = `http://localhost:${PORT}`;

async function testExecution() {
    let browser;
    try {
        browser = await puppeteer.launch({
            headless: 'new',
            args: [
                '--no-sandbox',
                '--disable-setuid-sandbox',
                '--disable-dev-shm-usage',
            ]
        });

        const page = await browser.newPage();

        // Capture console messages
        page.on('console', msg => {
            if (msg.text().includes('[useSimulator]') || msg.text().includes('[RESET]') || msg.text().includes('State updated')) {
                console.log(`[BROWSER] ${msg.text()}`);
            }
        });

        console.log('Loading page...');
        await page.goto(BASE_URL, { waitUntil: 'networkidle2', timeout: 30000 });

        // Wait for WASM to be ready
        await page.waitForFunction(() => window.Module !== undefined, { timeout: 10000 });
        console.log('✓ WASM module loaded');

        // Wait for simulator to be ready
        await page.waitForFunction(() => {
            return new Promise((resolve) => {
                const listener = (event) => {
                    if (event.data?.type === 'cpu-state') {
                        window.removeEventListener('message', listener);
                        resolve(event.data.state !== null);
                    }
                };
                window.addEventListener('message', listener);
                window.postMessage({ type: 'getState' });
                setTimeout(() => {
                    window.removeEventListener('message', listener);
                    resolve(false);
                }, 3000);
            });
        }, { timeout: 30000 });
        console.log('✓ Simulator ready');

        // Get initial state
        const initialState = await page.evaluate(() => {
            return new Promise((resolve) => {
                const listener = (event) => {
                    if (event.data?.type === 'cpu-state') {
                        window.removeEventListener('message', listener);
                        resolve(event.data.state);
                    }
                };
                window.addEventListener('message', listener);
                window.postMessage({ type: 'getState' });
                setTimeout(() => {
                    window.removeEventListener('message', listener);
                    resolve(null);
                }, 2000);
            });
        });

        console.log('\nInitial state after reset:');
        console.log(`  PC:  0x${initialState?.pc?.toString(16).padStart(6, '0')}`);
        console.log(`  SSP: 0x${initialState?.ssp?.toString(16).padStart(8, '0')}`);
        console.log(`  D0:  0x${initialState?.dregs?.[0]?.toString(16).padStart(8, '0')}`);

        // Press PLAY - run 10 instructions
        console.log('\nStarting execution (10 instructions)...');
        await page.evaluate(() => {
            window.postMessage({ type: 'run', payload: { count: 10 } });
        });

        // Wait a bit for execution to complete
        await new Promise(r => setTimeout(r, 1000));

        // Get state after execution
        const afterState = await page.evaluate(() => {
            return new Promise((resolve) => {
                const listener = (event) => {
                    if (event.data?.type === 'cpu-state') {
                        window.removeEventListener('message', listener);
                        resolve(event.data.state);
                    }
                };
                window.addEventListener('message', listener);
                window.postMessage({ type: 'getState' });
                setTimeout(() => {
                    window.removeEventListener('message', listener);
                    resolve(null);
                }, 2000);
            });
        });

        console.log('\nState after running 10 instructions:');
        console.log(`  PC:  0x${afterState?.pc?.toString(16).padStart(6, '0')}`);
        console.log(`  SSP: 0x${afterState?.ssp?.toString(16).padStart(8, '0')}`);
        console.log(`  D0:  0x${afterState?.dregs?.[0]?.toString(16).padStart(8, '0')}`);

        // Check progress
        if (afterState?.pc > initialState?.pc) {
            console.log(`\n✅ SUCCESS: PC incremented from 0x${initialState?.pc?.toString(16)} to 0x${afterState?.pc?.toString(16)}`);
        } else if (afterState?.pc === initialState?.pc) {
            console.log(`\n⚠️ PC did not increment (both 0x${afterState?.pc?.toString(16)})`);
        } else {
            console.log(`\n❌ PC went backwards: 0x${initialState?.pc?.toString(16)} → 0x${afterState?.pc?.toString(16)}`);
        }

    } catch (error) {
        console.error('Test failed:', error);
    } finally {
        if (browser) {
            await browser.close();
        }
    }
}

// Check if dev server is running
async function waitForServer(maxAttempts = 30) {
    for (let i = 0; i < maxAttempts; i++) {
        try {
            const response = await new Promise((resolve, reject) => {
                const req = http.get(BASE_URL, (res) => {
                    resolve(res.statusCode);
                });
                req.on('error', reject);
                req.setTimeout(1000);
            });
            if (response === 200) {
                console.log('✓ Dev server is running');
                return true;
            }
        } catch (e) {
            if (i < maxAttempts - 1) {
                console.log(`Waiting for dev server... (${i + 1}/${maxAttempts})`);
                await new Promise(r => setTimeout(r, 1000));
            }
        }
    }
    throw new Error('Dev server not responding');
}

async function main() {
    try {
        await waitForServer();
        await testExecution();
    } catch (error) {
        console.error('Fatal error:', error.message);
        process.exit(1);
    }
}

main();
