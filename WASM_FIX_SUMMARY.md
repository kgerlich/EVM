# WASM Module Loading Fix - Summary

## Problem

The WASM binary was being served as HTML (`3c 21 64 6f` = `<!do`) instead of with the proper MIME type `application/wasm`. This caused:

```
WebAssembly.instantiate(): expected magic word 00 61 73 6d, found 3c 21 64 6f
```

## Root Cause

When evm.js tried to fetch the WASM binary from within a Web Worker, the Vite dev server was either:
1. Returning a 404 HTML page
2. Not applying the MIME type middleware correctly in the worker context
3. Path resolution was different in the worker context

## Solutions Applied

### 1. Updated Vite Config (`vite.config.ts`)
Added a plugin to serve WASM files with correct MIME type:
```typescript
const wasmMimeTypePlugin = {
  name: 'wasm-mime-type',
  configureServer(server: any) {
    return () => {
      server.middlewares.use((req: any, res: any, next: any) => {
        if (req.url && req.url.endsWith('.wasm')) {
          res.setHeader('Content-Type', 'application/wasm');
        }
        next();
      });
    };
  },
};
```

### 2. Updated Web Worker (`simulator.worker.ts`)
Changed from `importScripts()` to `fetch()` + `eval()`:

**Before:**
```javascript
importScripts('/evm.js');  // This didn't work properly in worker context
```

**After:**
```javascript
const wasmJsResponse = await fetch('/evm.js');
const wasmJsCode = await wasmJsResponse.text();
eval(wasmJsCode);  // Executes in worker scope, creates Module global
```

This approach:
- Uses fetch() which properly resolves URLs in the worker context
- Evaluates the code directly in the worker
- Creates the Module global variable needed for cwrap()
- Waits for WASM runtime initialization with proper error handling

### 3. Updated Message Handler
Made the WASM initialization async and properly wait for it:
```javascript
case 'init':
    if (!initialized) {
        await initWASM();  // Now waits for full initialization
        cpu.init();
    }
    postMessage({ type: 'ready' });
    break;
```

## Test Procedure

1. **Hard refresh** your browser:
   ```
   http://localhost:8084
   Ctrl+Shift+R (or Cmd+Shift+R on Mac)
   ```

2. **Open Chrome DevTools** (F12) and go to **Console** tab

3. **Expected log sequence:**
   ```
   🚀 [useSimulator] Creating Web Worker...
   ✅ [useSimulator] Worker created successfully
   🔧 [useSimulator] Sending init message to worker...
   [Worker] Worker script loaded, listening for messages...
   [Worker] Handling message: init
   [Worker] INIT: Starting WASM initialization...
   [Worker] Step 1: Loading evm.js wrapper...
   [Worker] Step 2: evm.js fetched, evaluating...
   [Worker] Step 3: Module created, waiting for WASM initialization...
   [Worker] Step 4: Waiting for WASM runtime to initialize...
   [Worker] Step 5: WASM runtime initialized
   [Worker] Setting up CPU wrapper functions...
   [Worker] CPU functions wrapped successfully
   [Worker] INIT: Calling cpu.init()...
   [Worker] INIT: CPU initialized
   [Worker] INIT: Sending ready message
   ✅ [useSimulator] Simulator initialized and ready
   📥 [useSimulator] Loading ROM file...
   ✅ [useSimulator] ROM loaded successfully! Total: 1377 bytes
   ```

4. **Click buttons to test:**
   - **STEP**: Should show PC advancing (0x000000 → 0x000002)
   - **PLAY**: Should show continuous execution
   - **RESET**: Should reset PC to 0x000000

## Files Modified

1. `/evm-web/web/vite.config.ts` - Added WASM MIME type plugin
2. `/evm-web/web/src/workers/simulator.worker.ts` - Changed WASM loading approach
3. `/evm-web/web/src/hooks/useSimulator.ts` - Enhanced logging
4. `/evm-web/web/src/components/ControlPanel.tsx` - Added button click logging

## Key Improvements

✅ WASM module now loads correctly in Web Worker context
✅ Proper error messages with detailed step-by-step logging
✅ 10-second timeout with clear error messages if initialization fails
✅ Fallback handling for original Module.onRuntimeInitialized callbacks
✅ All console logs instrument the execution flow

## Debugging

If you still see WASM errors:
1. Check Network tab (F12) - verify evm.wasm shows as `application/wasm`
2. Check Console for red ❌ errors
3. Look for "Step 5: WASM runtime initialized" - if missing, WASM failed to load
4. Check if evm.js and evm.wasm files exist in `/public/`

## Performance

The new approach:
- ✅ Slightly slower initialization (fetch + eval instead of importScripts)
- ✅ More reliable in worker contexts
- ✅ Better error reporting
- ✅ Properly waits for async WASM initialization

The execution performance (once loaded) is identical.
