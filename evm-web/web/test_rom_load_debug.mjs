#!/usr/bin/env node

/**
 * Debug ROM loading to see exactly what's happening
 */

import puppeteer from 'puppeteer';
import http from 'http';
import fs from 'fs';

const PORT = 8084;
const BASE_URL = `http://localhost:${PORT}`;

async function testROMLoading() {
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

        // Capture ALL console messages
        page.on('console', msg => {
            console.log(`[BROWSER] ${msg.text()}`);
        });

        console.log('Loading page...');
        await page.goto(BASE_URL, { waitUntil: 'networkidle2', timeout: 30000 });

        // Wait for WASM to be ready
        await page.waitForFunction(() => window.Module !== undefined, { timeout: 10000 });
        console.log('✓ WASM module loaded');

        // Get initial state before ROM
        const beforeROM = await page.evaluate(() => {
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

        console.log('\nBefore ROM loading:');
        console.log(`  PC:  0x${beforeROM?.pc?.toString(16).padStart(6, '0') || 'null'}`);
        console.log(`  SSP: 0x${beforeROM?.ssp?.toString(16).padStart(6, '0') || 'null'}`);

        // Wait a bit for ROM to load
        console.log('\nWaiting for ROM to load...');
        await page.waitForTimeout(5000);

        // Get state after ROM should be loaded
        const afterROM = await page.evaluate(() => {
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

        console.log('\nAfter ROM loading:');
        console.log(`  PC:  0x${afterROM?.pc?.toString(16).padStart(6, '0') || 'null'}`);
        console.log(`  SSP: 0x${afterROM?.ssp?.toString(16).padStart(6, '0') || 'null'}`);

        // Check if ROM was actually loaded into memory by reading first bytes
        console.log('\nChecking memory at 0x000000:');
        const memoryTest = await page.evaluate(async () => {
            return new Promise((resolve) => {
                const listener = (event) => {
                    if (event.data?.type === 'memoryRead') {
                        window.removeEventListener('message', listener);
                        resolve(event.data.data);
                    }
                };
                window.addEventListener('message', listener);
                window.postMessage({
                    type: 'readMemory',
                    addr: 0x000000,
                    size: 16
                });
                setTimeout(() => {
                    window.removeEventListener('message', listener);
                    resolve(null);
                }, 2000);
            });
        });

        if (memoryTest) {
            console.log(`Memory bytes: ${memoryTest.slice(0, 8).join(' ')}`);
        } else {
            console.log('Failed to read memory');
        }

        // Summary
        if (afterROM?.pc === 0 && afterROM?.ssp === 0x410000) {
            console.log('\n⚠️ PC is 0 - ROM may not have loaded');
            console.log('SSP is default 0x410000 - reset vector not read from ROM');
        } else if (afterROM?.pc === 0xF0F0F0 || afterROM?.ssp === 0xF0F0F0) {
            console.log('\n❌ PC/SSP showing garbage (0xF0F0F0) - ROM not loaded');
        } else {
            console.log(`\n✓ Valid PC and SSP values`);
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
        await testROMLoading();
    } catch (error) {
        console.error('Fatal error:', error.message);
        process.exit(1);
    }
}

main();
