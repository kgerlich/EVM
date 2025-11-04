#!/usr/bin/env node

/**
 * Analyze PS20.S19 ROM structure
 * Shows what data is at the beginning of the ROM
 */

import fs from 'fs';

// Simplified S19 parser
function parseS19Line(line) {
    const trimmed = line.trim();
    if (!trimmed.startsWith('S') || trimmed.length < 4) {
        return null;
    }

    const type = trimmed.substring(0, 2);
    const byteCount = parseInt(trimmed.substring(2, 4), 16);

    if (byteCount * 2 + 4 !== trimmed.length) {
        return null;
    }

    const dataLen = byteCount - 1;
    let addressLen = 0;

    switch (type) {
        case 'S0': addressLen = 2; break;  // Header
        case 'S1': addressLen = 2; break;  // 16-bit address
        case 'S2': addressLen = 3; break;  // 24-bit address
        case 'S3': addressLen = 3; break;  // 24-bit address (68K style)
        case 'S7':
        case 'S8':
        case 'S9': return null;  // Termination
        default: return null;
    }

    const addressHex = trimmed.substring(4, 4 + addressLen * 2);
    const address = parseInt(addressHex, 16);

    const dataOffset = 4 + addressLen * 2;
    const dataLen2 = dataLen - addressLen;
    const dataHex = trimmed.substring(dataOffset, dataOffset + dataLen2 * 2);

    const data = new Uint8Array(dataLen2);
    for (let i = 0; i < dataLen2; i++) {
        data[i] = parseInt(dataHex.substring(i * 2, i * 2 + 2), 16);
    }

    return { type, address, data };
}

// Read and parse PS20.S19
const content = fs.readFileSync('/home/kgerlich/dev/EVM/evm-web/web/public/PS20.S19', 'utf8');
const lines = content.split('\n');

let minAddr = Infinity;
let maxAddr = 0;
let totalBytes = 0;
const segments = new Map();

console.log('=== Parsing PS20.S19 ===\n');

for (const line of lines) {
    const record = parseS19Line(line);
    if (record && record.type !== 'S0') {
        minAddr = Math.min(minAddr, record.address);
        maxAddr = Math.max(maxAddr, record.address + record.data.length - 1);
        totalBytes += record.data.length;
        segments.set(record.address, record.data);

        if (record.address < 0x100) {
            console.log(`S${record.type.substring(1)} @ 0x${record.address.toString(16).padStart(6, '0')}: ${record.data.length} bytes`);
            if (record.data.length <= 32) {
                let hex = '';
                for (let i = 0; i < record.data.length; i++) {
                    hex += record.data[i].toString(16).padStart(2, '0').toUpperCase() + ' ';
                }
                console.log(`         Data: ${hex}`);
            }
        }
    }
}

console.log(`\n=== Summary ===`);
console.log(`Total segments: ${segments.size}`);
console.log(`Total bytes: ${totalBytes}`);
console.log(`Address range: 0x${minAddr.toString(16).padStart(6, '0')} - 0x${maxAddr.toString(16).padStart(6, '0')}`);

// Show reset vectors area
console.log(`\n=== Reset Vectors Area (0x000000-0x00000F) ===`);
const firstSegment = segments.get(Math.min(...Array.from(segments.keys())));
if (firstSegment && firstSegment.length >= 8) {
    const ssp = (firstSegment[0] << 24) | (firstSegment[1] << 16) | (firstSegment[2] << 8) | firstSegment[3];
    const pc = (firstSegment[4] << 24) | (firstSegment[5] << 16) | (firstSegment[6] << 8) | firstSegment[7];

    console.log(`At 0x000000-0x000003 (SSP):  0x${ssp.toString(16).padStart(8, '0').toUpperCase()}`);
    console.log(`At 0x000004-0x000007 (PC):   0x${pc.toString(16).padStart(8, '0').toUpperCase()}`);

    if (ssp === 0 && pc === 0) {
        console.log('\n⚠️  Both reset vectors are ZERO - ROM may have invalid reset sequence');
    }
} else {
    console.log('⚠️  No data at 0x000000 - ROM segments start elsewhere');
}

// Show first few segments
console.log(`\n=== First 5 Segments ===`);
const sortedAddrs = Array.from(segments.keys()).sort((a, b) => a - b).slice(0, 5);
for (const addr of sortedAddrs) {
    const data = segments.get(addr);
    console.log(`  0x${addr.toString(16).padStart(6, '0')}: ${data.length} bytes`);
}
