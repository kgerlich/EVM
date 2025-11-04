#!/usr/bin/env node

import puppeteer from 'puppeteer';
import http from 'http';

const PORT = 8084;
const BASE_URL = `http://localhost:${PORT}`;

async function testJSR() {
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
            if (msg.text().includes('[useSimulator]') || msg.text().includes('[RESET]') || msg.text().includes('ROM loaded')) {
                console.log(`[BROWSER] ${msg.text()}`);
            }
        });

        console.log('Loading page...');
        await page.goto(BASE_URL, { waitUntil: 'networkidle2', timeout: 30000 });

        // Wait for WASM to be ready
        await page.waitForFunction(() => window.Module !== undefined, { timeout: 10000 });
        console.log('✓ WASM module loaded');

        // Wait for simulator to be ready (ROM loaded)
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

        // Get initial state (after reset, should be at PC=0x000008)
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

        console.log('\n📊 Initial state (PC=0x000008, should execute JSR 0x0000DB04):');
        console.log(`  PC:  0x${initialState?.pc?.toString(16).padStart(6, '0')}`);
        console.log(`  SSP: 0x${initialState?.ssp?.toString(16).padStart(8, '0')}`);
        console.log(`  A7:  0x${initialState?.aregs?.[7]?.toString(16).padStart(8, '0')}`);

        // Execute ONE instruction (the JSR at PC=0x000008)
        console.log('\n⚙️ Executing 1 instruction (JSR 0x4EB9)...');
        await page.evaluate(() => {
            return new Promise((resolve) => {
                const listener = (event) => {
                    if (event.data?.type === 'stepped') {
                        window.removeEventListener('message', listener);
                        resolve(true);
                    }
                };
                window.addEventListener('message', listener);
                window.postMessage({ type: 'step' });
                setTimeout(() => {
                    window.removeEventListener('message', listener);
                    resolve(false);
                }, 2000);
            });
        });

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

        console.log('\n📊 State after 1 instruction:');
        console.log(`  PC:  0x${afterState?.pc?.toString(16).padStart(6, '0')}`);
        console.log(`  SSP: 0x${afterState?.ssp?.toString(16).padStart(8, '0')}`);
        console.log(`  A7:  0x${afterState?.aregs?.[7]?.toString(16).padStart(8, '0')}`);

        // Check results
        console.log('\n🔍 Verification:');

        const expectedPC = 0x0000DB04;  // Target of JSR
        const actualPC = afterState?.pc;

        if (actualPC === expectedPC) {
            console.log(`✅ SUCCESS: PC jumped to 0x${actualPC.toString(16).padStart(6, '0')} (correct!)`);
        } else {
            console.log(`❌ FAILURE: PC is 0x${actualPC?.toString(16).padStart(6, '0')}, expected 0x${expectedPC.toString(16).padStart(6, '0')}`);
        }

        // Check stack pointer
        const expectedSP = initialState?.aregs?.[7] - 4;
        const actualSP = afterState?.aregs?.[7];

        if (actualSP === expectedSP) {
            console.log(`✅ SUCCESS: Stack pointer decremented correctly (0x${actualSP.toString(16).padStart(8, '0')})`);
        } else {
            console.log(`❌ FAILURE: Stack pointer is 0x${actualSP?.toString(16).padStart(8, '0')}, expected 0x${expectedSP?.toString(16).padStart(8, '0')}`);
        }

    } catch (error) {
        console.error('Test failed:', error.message);
        process.exit(1);
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
        await testJSR();
    } catch (error) {
        console.error('Fatal error:', error.message);
        process.exit(1);
    }
}

main();
