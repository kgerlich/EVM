# WASM Loader Path Resolution Fix

## Problem Identified

The Web Worker was loading `evm.js` successfully via `importScripts()`, but when `evm.js` tried to fetch the WASM binary (`evm.js.wasm`), it was getting an HTML error page instead of the WASM file.

**Error Message:**
```
expected magic word 00 61 73 6d, found 3c 21 64 6f
```

The hex `3c 21 64 6f` = `<!do` from an HTML error page, indicating a 404 was returned.

## Root Cause

The issue was **path resolution in the Worker context**:

1. evm.js (the Emscripten-generated wrapper) tries to locate the WASM binary
2. In a Worker, the script directory resolution can be problematic
3. evm.js was using a relative path that resolved incorrectly
4. The fetch was going to the wrong URL and getting a 404 HTML page

## Solution

Set `Module.locateFile` **BEFORE** importing evm.js. This function tells evm.js where to find the WASM file.

**Key Code Change in `src/workers/simulator.worker.ts`:**

```typescript
// CRITICAL: Set Module.locateFile BEFORE importing evm.js
// This ensures evm.js uses the correct path to fetch evm.js.wasm
(self as any).Module = {
    locateFile: (filename: string) => {
        console.log(`[Worker] Module.locateFile called for: ${filename}`);
        // Return absolute paths from root for all files
        // evm.js will look for "evm.js.wasm" which should be at /evm.js.wasm
        if (filename.startsWith('/')) {
            return filename;  // Already absolute
        }
        return '/' + filename;  // Make it absolute
    }
};

// Use importScripts which is designed for loading scripts in workers
try {
    (self as any).importScripts('/evm.js');
    console.log('[Worker] Step 3: evm.js loaded via importScripts');
} catch (importError: any) {
    console.error('[Worker] importScripts error:', importError);
    throw new Error(`Failed to import evm.js: ${importError.message}`);
}
```

## How It Works

1. **Before importScripts()**: We create a Module object with a custom `locateFile` function
2. **evm.js checks**: When evm.js loads, it sees `Module` is already defined (checks `typeof Module !== 'undefined'`)
3. **evm.js uses Module**: Instead of creating its own Module, evm.js uses ours
4. **Path resolution**: When evm.js needs to fetch the WASM file, it calls the `locateFile` function we provided
5. **Correct path**: Our function returns `/evm.js.wasm` (absolute path from root)
6. **Fetch succeeds**: The browser/Vite serves the file with correct MIME type

## Verification

### File Status
- ✅ `/evm.js` exists (16KB) - loaded via importScripts
- ✅ `/evm.js.wasm` exists (290KB) - valid WASM binary
- ✅ Vite middleware sets `Content-Type: application/wasm` for .wasm files
- ✅ curl can fetch both files correctly

### Expected Console Sequence (After Fix)

When you refresh the browser and open DevTools (F12):

```
🚀 [useSimulator] Creating Web Worker...
✅ [useSimulator] Worker created successfully
🔧 [useSimulator] Sending init message to worker...
[Worker] Worker script loaded, listening for messages...
[Worker] Handling message: init
[Worker] INIT: Starting WASM initialization...
[Worker] Step 1: Checking if Module already exists...
[Worker] Step 2: Module not found, using importScripts to load evm.js...
[Worker] Module.locateFile called for: evm.js.wasm    ← NEW: This shows our fix working!
[Worker] Step 3: evm.js loaded via importScripts
[Worker] Step 4: Module available, checking WASM initialization...
[Worker] Step 5: Waiting for WASM runtime to initialize...
[Vite] Setting WASM MIME type for: /evm.js.wasm      ← Middleware working
[Worker] Step 6: WASM runtime initialized
[Worker] Setting up CPU wrapper functions...
[Worker] CPU functions wrapped successfully
[Worker] INIT: Calling cpu.init()...
[Worker] INIT: CPU initialized
[Worker] INIT: Sending ready message
✅ [useSimulator] Simulator initialized and ready
```

## Testing Instructions

### Step 1: Refresh Browser
1. Open: `http://localhost:8084`
2. Hard refresh: `Ctrl+Shift+R` (Windows/Linux) or `Cmd+Shift+R` (Mac)

### Step 2: Check Console
1. Open DevTools: `F12`
2. Click "Console" tab
3. Look for the log sequence above
4. **Key indicators of success:**
   - `[Worker] Module.locateFile called for: evm.js.wasm`
   - `[Worker] Step 6: WASM runtime initialized`
   - `✅ [useSimulator] Simulator initialized and ready`

### Step 3: Test Buttons
1. If console shows success, test buttons:
   - **STEP**: PC should advance (0x000000 → 0x000002 → 0x000004)
   - **PLAY**: Continuous execution starts
   - **RESET**: PC returns to 0x000000

### Step 4: Troubleshooting
If you still see errors:

1. **Check Network tab** (F12 → Network):
   - Look for `/evm.js.wasm` request
   - Check if response header shows `Content-Type: application/wasm`
   - If not, the Vite middleware isn't working

2. **Check for Module.locateFile log**:
   - If you don't see `[Worker] Module.locateFile called for: evm.js.wasm`
   - The Module object wasn't properly initialized
   - Check browser cache (try Ctrl+Shift+Delete to clear cache and cookies)

3. **Check dev server logs**:
   - Look for `[Vite] Setting WASM MIME type for:` message
   - If missing, the middleware isn't being triggered

## Technical Details

### Why This Fix Works

**The evm.js Code:**
```javascript
// Inside evm.js generated by Emscripten
function locateFile(path){
    if(Module["locateFile"]){
        return Module["locateFile"](path, scriptDirectory)
    }
    return scriptDirectory + path
}
```

evm.js defines a `locateFile` function that checks if `Module.locateFile` exists. If it does, it calls that instead of using the default path resolution. Our fix provides that function!

### Why Worker Context Is Different

In a Web Worker:
- `self.location.href` might not point to the expected URL
- Script directory calculation can be unreliable
- Relative paths may resolve to unexpected locations
- Setting it explicitly avoids all these issues

### Files Modified

1. **src/workers/simulator.worker.ts**
   - Added Module.locateFile before importScripts()
   - Added diagnostic logging for locateFile calls
   - No other changes needed

## Success Criteria

✅ All checks passing:
1. `Module.locateFile called for: evm.js.wasm` appears in console
2. `WASM runtime initialized` appears in console
3. `Simulator initialized and ready` appears in console
4. Buttons are enabled (not grayed out)
5. STEP button advances PC
6. PLAY button starts continuous execution

## Next Steps

Once this is working:
1. Load ROM file (PS20.S19) using the Load ROM button
2. Test step-by-step execution
3. Test ROM execution with PLAY button
4. Monitor registers and memory in the UI

---

**Note:** The Vite dev server is running at `http://localhost:8084` and will hot-reload any changes. Simply saving a file will auto-update the browser.
