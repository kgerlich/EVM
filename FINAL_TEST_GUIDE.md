# Final Test Guide - EVM Simulator

## What Changed

1. **Vite Config** - Fixed WASM MIME type middleware to apply to all `.wasm` file requests
2. **Worker Initialization** - Using `importScripts()` (the proper way for Web Workers)
3. **Full Logging** - Every step is logged to console for debugging

## Test Procedure

### Step 1: Hard Refresh Browser
```
URL: http://localhost:8084
Keys: Ctrl+Shift+R  (Windows/Linux)
      Cmd+Shift+R   (Mac)
```

### Step 2: Open DevTools Console
```
Keys: F12
Then: Click "Console" tab
```

### Step 3: Expected Log Sequence

You should see these logs in order:

```
🚀 [useSimulator] Creating Web Worker...
✅ [useSimulator] Worker created successfully
🔧 [useSimulator] Sending init message to worker...
🛑 [useSimulator] Terminating worker        ← This is cleanup from old connections, OK

🚀 [useSimulator] Creating Web Worker...
✅ [useSimulator] Worker created successfully
🔧 [useSimulator] Sending init message to worker...

[Worker] Worker script loaded, listening for messages...
[Worker] Handling message: init
[Worker] INIT: Starting WASM initialization...
[Worker] Step 1: Checking if Module already exists...
[Worker] Step 2: Module not found, using importScripts to load evm.js...
[Worker] Step 3: evm.js loaded via importScripts         ← KEY LINE 1
[Worker] Step 4: Module available, checking WASM initialization...
[Worker] Step 5: Waiting for WASM runtime to initialize...

[Vite] Setting WASM MIME type for: /evm.wasm           ← KEY LINE 2 (shows Vite middleware working)

[Worker] Step 6: WASM runtime initialized              ← KEY LINE 3 (SUCCESS!)
[Worker] Setting up CPU wrapper functions...
[Worker] CPU functions wrapped successfully
[Worker] INIT: Calling cpu.init()...
[Worker] INIT: CPU initialized
[Worker] INIT: Sending ready message

✅ [useSimulator] Simulator initialized and ready       ← MAJOR SUCCESS!
📥 [useSimulator] Loading ROM file...
✅ [useSimulator] Fetched PS20.S19, status: 200
✅ [useSimulator] ROM content length: 4168 bytes
🔍 [useSimulator] Parsing S19 format...
✅ [useSimulator] Parsed 92 memory segments
📍 [useSimulator] Loading segment at 0x000000, size: 15 bytes
[Worker] Handling message: loadROM
[Worker] LOADROM: Loading 15 bytes
[Worker] LOADROM: Success
... (more segments loading) ...
✅ [useSimulator] ROM loaded successfully! Total: 1377 bytes    ← ROM READY!
```

### Step 4: Test the Buttons

#### STEP Button
1. Click **STEP** button
2. Should see in console:
```
🎮 [ControlPanel] STEP button clicked
⏭️ [useSimulator] Stepping one instruction
[Worker] Handling message: step
[Worker] STEP: Executing one instruction
[Worker] STEP: Done, PC=0x000002      ← PC ADVANCED!
📊 [useSimulator] CPU State updated: {
    pc: "0x000002",
    sr: "0x2700",
    d0: "0x00000000",
    a7: "0x00041fff"
}
```

#### Click STEP Again
3. PC should advance further (0x000004, 0x000006, etc.)
4. Each click shows a new PC value

#### PLAY Button
1. Click **PLAY** button
2. Should see:
```
🎮 [ControlPanel] PLAY button clicked
▶️ [useSimulator] Running 1000 instructions
[Worker] Handling message: run
[Worker] RUN: Executing 1000 instructions
[Worker] RUN: Done, PC=0xXXXXXX
📊 [useSimulator] CPU State updated: {...}
```
3. CPU will continue executing until you click PAUSE
4. PC will keep advancing

#### PAUSE Button
1. Click **PAUSE** button
2. Should see:
```
🎮 [ControlPanel] PAUSE button clicked
⏸️ [useSimulator] Pausing execution
[Worker] Handling message: pause
[Worker] PAUSE: Pausing CPU
```

#### RESET Button
1. Click **RESET** button
2. Should see:
```
🎮 [ControlPanel] RESET button clicked
🔄 [useSimulator] Resetting CPU
[Worker] Handling message: reset
[Worker] RESET: Resetting CPU
[Worker] RESET: Done, PC=0x000000     ← PC back to start!
📊 [useSimulator] CPU State updated: {
    pc: "0x000000",    ← KEY: Should be 0x000000
    ...
}
```

---

## Success Criteria

✅ **All of the following MUST be true:**

1. **Initialization:**
   - See "[Worker] Step 3: evm.js loaded via importScripts"
   - See "[Vite] Setting WASM MIME type for: /evm.wasm"
   - See "[Worker] Step 6: WASM runtime initialized"
   - See "✅ [useSimulator] Simulator initialized and ready"

2. **ROM Loading:**
   - See "✅ [useSimulator] ROM loaded successfully!"
   - Total bytes shown (should be 1377)

3. **STEP Execution:**
   - PC advances (0x000000 → 0x000002 → 0x000004, etc.)
   - Registers update

4. **PLAY/PAUSE:**
   - PLAY starts continuous execution
   - PC keeps advancing
   - PAUSE stops it

5. **RESET:**
   - Clicking RESET sets PC back to 0x000000

---

## Debugging - If Something Goes Wrong

### Error: "Module not found after importScripts"
- Check "[Vite] Setting WASM MIME type" log
- If MISSING: The Vite middleware isn't being called
- Solution: Check Network tab (F12) - does /evm.wasm show as `application/wasm`?

### Error: "WASM runtime initialized" NOT appearing
- The WASM failed to load
- Check browser Network tab for 404 errors
- Verify /evm.wasm and /evm.js files are served
- Check if any red error messages appear in console

### PC Not Advancing
- Verify "ROM loaded successfully" appears
- Check if STEP button is actually enabled (not grayed out)
- Try clicking RESET first, then STEP

### Buttons are Grayed Out/Disabled
- Check "Simulator initialized and ready" log
- If missing: WASM initialization failed
- Check all red ❌ errors in console

---

## Quick Diagnostics

Copy and paste into console to check state:

```javascript
// Check if simulator is initialized
console.log('Looking for logs...');

// Filter logs to see only errors
console.error = console.log;  // Redirect errors to log
```

Or check Network tab (F12):
1. Go to Network tab
2. Look for requests to:
   - `/evm.js` (should see it)
   - `/evm.wasm` (should see it with MIME type `application/wasm`)
   - `/PS20.S19` (ROM file)

---

## Next Steps If Everything Works

✅ **Congratulations!** Your EVM simulator is fully functional!

You can now:
1. **Step through instructions** - Click STEP repeatedly
2. **Run continuously** - Click PLAY to execute
3. **Monitor CPU state** - Watch registers update in real-time
4. **Test the ROM** - PS20.S19 is being executed

---

## Files Changed (For Reference)

1. `/evm-web/web/vite.config.ts` - Fixed WASM MIME type middleware
2. `/evm-web/web/src/workers/simulator.worker.ts` - Using proper `importScripts()`
3. `/evm-web/web/src/hooks/useSimulator.ts` - Enhanced logging
4. `/evm-web/web/src/components/ControlPanel.tsx` - Button logging

All changes are automatically applied via Vite's hot module replacement (HMR).

---

**Test it now and share console output if you see any errors!** 🚀
