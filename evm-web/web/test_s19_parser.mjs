#!/usr/bin/env node

import fs from 'fs';

// Simplified S19 parser to debug
function parseS19(content) {
    const lines = content.split('\n');
    const segments = new Map();
    let recordCount = 0;
    let recordTypes = { S0: 0, S1: 0, S2: 0, S3: 0, S5: 0, S7: 0, S8: 0, S9: 0 };

    for (const line of lines) {
        const trimmed = line.trim();
        if (!trimmed.startsWith('S') || trimmed.length < 4) continue;

        const type = trimmed.substring(0, 2);
        recordTypes[type] = (recordTypes[type] || 0) + 1;

        if (type !== 'S3') continue; // Only process S3

        recordCount++;
        const byteCountStr = trimmed.substring(2, 4);
        const byteCount = parseInt(byteCountStr, 16);

        const addressHex = trimmed.substring(4, 10); // 3 bytes = 6 hex chars
        const address = parseInt(addressHex, 16);

        const dataStartIdx = 10;
        const dataEndIdx = trimmed.length - 2; // -2 for checksum
        const dataLen = dataEndIdx - dataStartIdx;
        const dataBytes = dataLen / 2;

        if (!segments.has(address)) {
            segments.set(address, []);
        }
        segments.get(address).push(dataBytes);
    }

    console.log('\n=== S19 Parse Results ===');
    console.log('Record counts:', recordTypes);
    console.log(`Total S3 records: ${recordCount}`);
    console.log(`Unique addresses: ${segments.size}`);

    let totalDataBytes = 0;
    const addresses = Array.from(segments.keys()).sort((a, b) => a - b);

    console.log('\nFirst 20 addresses:');
    for (let i = 0; i < Math.min(20, addresses.length); i++) {
        const addr = addresses[i];
        const dataLengths = segments.get(addr);
        const totalAtAddr = dataLengths.reduce((a, b) => a + b, 0);
        totalDataBytes += totalAtAddr;
        console.log(`  0x${addr.toString(16).padStart(6, '0')}: ${dataLengths.length} record(s), ${totalAtAddr} total bytes`);
    }

    if (addresses.length > 20) {
        console.log(`  ... and ${addresses.length - 20} more addresses`);
        for (let i = addresses.length - 5; i < addresses.length; i++) {
            const addr = addresses[i];
            const dataLengths = segments.get(addr);
            const totalAtAddr = dataLengths.reduce((a, b) => a + b, 0);
            totalDataBytes += totalAtAddr;
            console.log(`  0x${addr.toString(16).padStart(6, '0')}: ${dataLengths.length} record(s), ${totalAtAddr} total bytes`);
        }
    } else {
        // Total was already calculated above
        totalDataBytes = 0;
        for (const addr of addresses) {
            const dataLengths = segments.get(addr);
            totalDataBytes += dataLengths.reduce((a, b) => a + b, 0);
        }
    }

    console.log(`\nEstimated total data bytes: ${totalDataBytes}`);
    return { segments, recordCount, totalDataBytes };
}

const ps20 = fs.readFileSync('/home/kgerlich/dev/EVM/evm-web/web/public/PS20.S19', 'utf8');
parseS19(ps20);
