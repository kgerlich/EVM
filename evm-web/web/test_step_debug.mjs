#!/usr/bin/env node

import puppeteer from 'puppeteer';
import http from 'http';

const PORT = 8084;
const BASE_URL = `http://localhost:${PORT}`;

async function testStep() {
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
            const text = msg.text();
            if (text.includes('State updated') || text.includes('RESET') || text.includes('[useSimulator]')) {
                console.log(`[BROWSER] ${text}`);
            }
        });

        console.log('Loading page...');
        await page.goto(BASE_URL, { waitUntil: 'networkidle2', timeout: 30000 });

        // Wait for simulator ready
        await page.waitForFunction(() => window.Module !== undefined, { timeout: 10000 });
        console.log('✓ WASM ready\n');

        // Wait for initialization complete
        await new Promise(r => setTimeout(r, 8000));

        // Get state after reset
        const resetState = await page.evaluate(() => {
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

        console.log('After RESET:');
        console.log(`  PC = 0x${resetState?.pc?.toString(16).padStart(6, '0')}`);
        console.log(`  SR = 0x${resetState?.sr?.toString(16).padStart(4, '0')}`);
        console.log(`  D0 = 0x${resetState?.dregs?.[0]?.toString(16).padStart(8, '0')}`);
        console.log('');

        // Now step ONE instruction
        console.log('Calling STEP (1 instruction)...\n');
        await page.evaluate(() => {
            window.postMessage({ type: 'step' });
        });

        // Wait for step to complete
        await new Promise(r => setTimeout(r, 500));

        // Get state after step
        const afterStep = await page.evaluate(() => {
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

        console.log('After STEP:');
        console.log(`  PC = 0x${afterStep?.pc?.toString(16).padStart(6, '0')}`);
        console.log(`  SR = 0x${afterStep?.sr?.toString(16).padStart(4, '0')}`);
        console.log(`  D0 = 0x${afterStep?.dregs?.[0]?.toString(16).padStart(8, '0')}`);
        console.log('');

        if (afterStep?.pc < resetState?.pc) {
            console.log(`❌ ERROR: PC went BACKWARDS from 0x${resetState?.pc?.toString(16)} to 0x${afterStep?.pc?.toString(16)}`);
            console.log('This suggests an exception or illegal instruction at the reset PC');
        } else if (afterStep?.pc === resetState?.pc) {
            console.log(`⚠️ PC did not advance (stuck at 0x${afterStep?.pc?.toString(16)})`);
        } else {
            console.log(`✓ PC advanced: 0x${resetState?.pc?.toString(16)} → 0x${afterStep?.pc?.toString(16)}`);
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
                console.log('✓ Dev server is running\n');
                return true;
            }
        } catch (e) {
            if (i < maxAttempts - 1) {
                process.stdout.write('.');
                await new Promise(r => setTimeout(r, 1000));
            }
        }
    }
    throw new Error('Dev server not responding');
}

async function main() {
    try {
        await waitForServer();
        await testStep();
    } catch (error) {
        console.error('Fatal error:', error.message);
        process.exit(1);
    }
}

main();
