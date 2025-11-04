# ✅ WASM Loading Fix Applied

## Summary of the Problem

**User's Error:**
```
expected magic word 00 61 73 6d, found 3c 21 64 6f
Aborted(CompileError: WebAssembly.instantiate(): expected magic word 00 61 73 6d, found 3c 21 64 6f @+0)
```

**What This Meant:**
- evm.js tried to load `evm.js.wasm` via fetch in the Web Worker
- Instead of getting the WASM binary (magic `00 61 73 6d`), it got HTML (`3c 21 64 6f` = `<!do`)
- The HTML was a 404 error page, indicating the fetch failed

**Root Cause:**
Path resolution in Web Worker context is problematic. When evm.js tried to fetch the WASM file using a relative or script-directory-relative path, the fetch went to the wrong URL and got a 404 error page instead of the WASM binary.

## The Fix (Applied to simulator.worker.ts)

### Part 1: Module Configuration (Before importScripts)

```typescript
(self as any).Module = {
    locateFile: (filename: string, scriptDirectory?: string) => {
        const resolvedPath = '/' + filename;
        console.log(`[Worker] Module.locateFile: "${filename}" → "${resolvedPath}"`);
        return resolvedPath;
    },
    print: (text: string) => console.log('[WASM stdout]', text),
    printErr: (text: string) => console.error('[WASM stderr]', text),
};
```

**Purpose:** Tells evm.js where to find asset files (like evm.js.wasm) by returning absolute paths from the root.

### Part 2: Pre-load WASM Binary

```typescript
console.log('[Worker] Attempting to fetch WASM binary pre-emptively...');
try {
    const wasmResponse = await fetch('/evm.js.wasm', { credentials: 'same-origin' });
    if (wasmResponse.ok) {
        const wasmBuffer = await wasmResponse.arrayBuffer();
        (self as any).Module.wasmBinary = new Uint8Array(wasmBuffer);
        console.log(`[Worker] Successfully pre-loaded WASM binary (${wasmBuffer.byteLength} bytes)`);
    }
} catch (error) {
    console.warn('[Worker] Failed to pre-load WASM:', error);
    // Continue anyway, evm.js will try to fetch it
}
```

**Purpose:** Fetch the WASM binary upfront and attach it to `Module.wasmBinary`. When evm.js loads, it will see that the binary is already available and use it instead of trying to fetch it.

## How This Solves The Problem

**Original Flow (Broken):**
```
evm.js needs WASM
  ↓
Tries to calculate WASM path (using script directory)
  ↓
Path calculation fails/returns wrong path
  ↓
fetch() gets 404 error page (HTML)
  ↓
WebAssembly.instantiate() gets HTML instead of WASM
  ↓
ERROR: "expected magic word 00 61 73 6d, found 3c 21 64 6f"
```

**New Flow (Fixed):**
```
Worker creates Module with locateFile
  ↓
Worker pre-fetches /evm.js.wasm
  ↓
Worker attaches binary to Module.wasmBinary
  ↓
importScripts('/evm.js') loads evm.js
  ↓
evm.js checks if Module.wasmBinary exists
  ↓
YES → evm.js uses pre-loaded binary
  ↓
WebAssembly.instantiate() gets valid WASM
  ↓
SUCCESS: WASM runtime initializes
```

## Files Modified

**`/evm-web/web/src/workers/simulator.worker.ts`**
- Added Module.locateFile before importScripts
- Added Module.wasmBinary pre-loading
- Added diagnostic console logging
- No other changes needed

## How to Test

### Quick Test (30 seconds)

1. **Refresh Browser:**
   - Go to `http://localhost:8084`
   - Press `Ctrl+Shift+R` (hard refresh)

2. **Open DevTools:**
   - Press `F12`
   - Click "Console" tab

3. **Look for:**
   ```
   [Worker] Successfully pre-loaded WASM binary (280000 bytes)
   [Worker] WASM runtime initialized
   ✅ [useSimulator] Simulator initialized and ready
   ```

4. **Test Buttons:**
   - STEP should advance PC
   - PLAY should run continuously
   - RESET should return PC to 0x000000

### Detailed Diagnostics

If buttons still don't work:

1. **Check if WASM loaded:**
   ```
   Search console for: "Successfully pre-loaded WASM binary"
   ```
   - If found: WASM loading works
   - If not found: Check next point

2. **Check HTTP response:**
   - DevTools → Network tab
   - Look for `/evm.js.wasm` request
   - Check Response header: `Content-Type: application/wasm`
   - If shows `text/html`: Vite middleware issue

3. **Clear cache:**
   - `Ctrl+Shift+Delete` (clear all)
   - Close browser completely
   - Reopen http://localhost:8084

4. **Check Vite is running:**
   - Terminal: `ps aux | grep vite | grep 8084`
   - Should show running vite process

## Expected Console Output

After hard refresh, you should see:

```
🚀 [useSimulator] Creating Web Worker...
✅ [useSimulator] Worker created successfully
🔧 [useSimulator] Sending init message to worker...
[Worker] Worker script loaded, listening for messages...
[Worker] Handling message: init
[Worker] INIT: Starting WASM initialization...
[Worker] Step 1: Checking if Module already exists...
[Worker] Step 2: Module not found, using importScripts to load evm.js...
[Worker] Creating Module with locateFile...
[Worker] Module created with locateFile
[Worker] Attempting to fetch WASM binary pre-emptively...
[Worker] Successfully pre-loaded WASM binary (280888 bytes)         ← SUCCESS!
[Worker] Step 3: evm.js loaded via importScripts
[Worker] Step 4: Module available, checking WASM initialization...
[Worker] Step 5: Waiting for WASM runtime to initialize...
[Vite] Setting WASM MIME type for: /evm.js.wasm
[Worker] Step 6: WASM runtime initialized                           ← WASM WORKS!
[Worker] Setting up CPU wrapper functions...
[Worker] CPU functions wrapped successfully
[Worker] INIT: Calling cpu.init()...
[Worker] INIT: CPU initialized
[Worker] INIT: Sending ready message
✅ [useSimulator] Simulator initialized and ready                    ← READY!
```

## Success Criteria

All of these should be true:

- ✅ Console shows "Successfully pre-loaded WASM binary"
- ✅ Console shows "WASM runtime initialized"
- ✅ Console shows "Simulator initialized and ready"
- ✅ All buttons are enabled (not grayed out)
- ✅ STEP button advances the PC
- ✅ PLAY button starts execution
- ✅ RESET button resets PC to 0x000000

## Why This Works

1. **Module.locateFile** - evm.js calls this to get file paths. We return absolute paths (`/evm.js.wasm`) so fetch knows exactly where to look.

2. **Pre-loaded Binary** - By fetching the WASM ourselves before evm.js loads, we guarantee:
   - The binary is loaded from a known, absolute path
   - We control the fetch and can log success/failure
   - evm.js uses the pre-loaded binary instead of trying to fetch it again
   - No path resolution ambiguity

3. **Emscripten Module Pattern** - evm.js (generated by Emscripten) is designed to accept pre-loaded binaries via `Module.wasmBinary`. This is a standard pattern.

## Technical Details

The evm.js code checks:
```javascript
if(Module["wasmBinary"])wasmBinary=Module["wasmBinary"];
```

So when we set `Module.wasmBinary`, evm.js automatically uses it. This is explicitly supported by Emscripten!

## Next Steps

1. **Test the fix** - Follow the "Quick Test" section above
2. **Load ROM** - Use Load ROM button to test S19 file loading
3. **Run simulation** - Test STEP, PLAY, PAUSE, RESET buttons
4. **Monitor execution** - Watch console logs and CPU state updates

---

**The fix is ready. Refresh your browser and test!**
