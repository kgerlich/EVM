/**
 * S19 (Motorola S-record) file parser
 *
 * Converts Motorola S-record format to memory writes
 * Matches original s19.c logic: parse each record and write bytes to address
 */

export interface S19Record {
    type: string;
    address: number;
    data: Uint8Array;
    checksum: number;
}

export interface S19ParseResult {
    records: S19Record[];
    segments: Map<number, Uint8Array>;
    error?: string;
}

/**
 * Parse a single S19 record line
 * Format examples:
 * S0: Header record (contains description)
 * S1: 16-bit address records
 * S2: 24-bit address records
 * S3: 32-bit address records (but we mask to 24-bit for 68k)
 */
function parseRecord(line: string): S19Record | null {
    const trimmed = line.trim();

    if (!trimmed.startsWith('S') || trimmed.length < 4) {
        return null;
    }

    const type = trimmed[0] + trimmed[1]; // e.g., "S3"
    const byteCountStr = trimmed.substring(2, 4);
    const byteCount = parseInt(byteCountStr, 16);

    if (byteCount * 2 + 4 !== trimmed.length) {
        return null;
    }

    const dataLen = byteCount - 1; // Exclude checksum byte
    let addressLen = 0;
    let dataOffset = 4;

    switch (type) {
        case 'S0':
            addressLen = 2; // 16-bit address (header)
            break;
        case 'S1':
            addressLen = 2; // 16-bit address
            break;
        case 'S2':
            addressLen = 3; // 24-bit address
            break;
        case 'S3':
            addressLen = 4; // 32-bit address, but mask to 24-bit for 68K
            break;
        case 'S7':
        case 'S8':
        case 'S9':
            // Termination records - skip
            return null;
        default:
            return null;
    }

    const addressBytes = trimmed.substring(dataOffset, dataOffset + addressLen * 2);
    let address = parseInt(addressBytes, 16);

    // Mask to 24-bit for 68K (S3 records can be 32-bit)
    address = address & 0xFFFFFF;

    dataOffset += addressLen * 2;
    const dataHex = trimmed.substring(dataOffset, dataOffset + (dataLen - addressLen) * 2);
    const checksumStr = trimmed.substring(dataOffset + (dataLen - addressLen) * 2, dataOffset + (dataLen - addressLen) * 2 + 2);

    const data = new Uint8Array((dataLen - addressLen) / 2);
    for (let i = 0; i < data.length; i++) {
        data[i] = parseInt(dataHex.substring(i * 2, i * 2 + 2), 16);
    }

    const checksum = parseInt(checksumStr, 16);

    return {
        type,
        address,
        data,
        checksum
    };
}

/**
 * Parse S19 file content and build a memory map
 * Returns a map where each key is a starting address and value is the data at that address
 */
export function parseS19(content: string): S19ParseResult {
    const lines = content.split('\n');
    const records: S19Record[] = [];
    const segments = new Map<number, Uint8Array>();
    let s3RecordCount = 0;

    for (const line of lines) {
        const record = parseRecord(line);
        if (record && (record.type === 'S0' || record.type === 'S1' || record.type === 'S2' || record.type === 'S3')) {
            records.push(record);

            // For data records (not headers), build memory map
            if (record.data.length > 0 && record.type !== 'S0') {
                if (record.type === 'S3') s3RecordCount++;

                // Each record represents data at a specific address
                // Multiple records at same address should be concatenated (sequential writes to same address)
                if (segments.has(record.address)) {
                    const existing = segments.get(record.address)!;
                    const combined = new Uint8Array(existing.length + record.data.length);
                    combined.set(existing);
                    combined.set(record.data, existing.length);
                    segments.set(record.address, combined);
                } else {
                    segments.set(record.address, record.data);
                }
            }
        }
    }

    // Debug logging
    let totalBytes = 0;
    for (const [addr, data] of segments) {
        totalBytes += data.length;
    }
    console.log(`[S19Parser] Parsed ${s3RecordCount} S3 records into ${segments.size} segments, ${totalBytes} total bytes`);

    return { records, segments };
}

/**
 * Merge overlapping/adjacent segments into continuous memory blocks
 * Note: NOT used by default. Instead, load segments one-by-one to memory via WASM.
 * This function is kept for reference but not needed for correct S19 loading.
 */
export function mergeSegments(segments: Map<number, Uint8Array>): Map<number, Uint8Array> {
    const merged = new Map<number, Uint8Array>();

    if (segments.size === 0) {
        return merged;
    }

    // Sort addresses
    const addresses = Array.from(segments.keys()).sort((a, b) => a - b);

    let currentAddr = addresses[0];
    let currentData = new Uint8Array(segments.get(currentAddr)!);

    for (let i = 1; i < addresses.length; i++) {
        const nextAddr = addresses[i];
        const nextData = segments.get(nextAddr)!;

        const currentEnd = currentAddr + currentData.length;

        // Only merge if segments are truly adjacent or overlapping (no gaps)
        if (nextAddr <= currentEnd) {
            // Overlapping or adjacent
            if (nextAddr + nextData.length > currentEnd) {
                // Extend current segment with new data beyond current end
                const newLength = nextAddr - currentAddr + nextData.length;
                const newData = new Uint8Array(newLength);
                newData.set(currentData);
                newData.set(nextData, nextAddr - currentAddr);
                currentData = newData;
            }
            // else: nextData is fully contained in currentData, skip
        } else {
            // Gap found - don't merge, save current and start new
            merged.set(currentAddr, currentData);
            currentAddr = nextAddr;
            currentData = new Uint8Array(nextData);
        }
    }

    merged.set(currentAddr, currentData);

    let totalBytes = 0;
    for (const data of merged.values()) {
        totalBytes += data.length;
    }
    console.log(`[S19Parser] After merge: ${merged.size} segments, ${totalBytes} total bytes`);

    return merged;
}
