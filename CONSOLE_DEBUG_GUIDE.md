# Chrome DevTools Console Debug Guide

## How to View Console Logs

1. Open Chrome DevTools: **F12** or **Right-click → Inspect**
2. Click the **Console** tab
3. You'll see detailed logs as the simulator runs

---

## What You'll See on Page Load

### 1. Worker Initialization
```
🚀 [useSimulator] Creating Web Worker...
✅ [useSimulator] Worker created successfully
🔧 [useSimulator] Sending init message to worker...
📨 [useSimulator] Received message from worker: init
[Worker] Worker script loaded, listening for messages...
[Worker] Handling message: init
[Worker] INIT: Starting WASM initialization...
[Worker] Loading WASM module via importScripts...
[Worker] WASM module loaded, cwrap available: function
[Worker] WASM already initialized, setting up CPU
[Worker] Setting up CPU wrapper functions...
[Worker] CPU functions wrapped successfully
[Worker] INIT: Calling cpu.init()...
[Worker] INIT: CPU initialized
[Worker] INIT: Sending ready message
✅ [useSimulator] Simulator initialized and ready
```

### 2. ROM Auto-Loading
```
📥 [useSimulator] Loading ROM file...
✅ [useSimulator] Fetched PS20.S19, status: 200
✅ [useSimulator] ROM content length: 4168 bytes
🔍 [useSimulator] Parsing S19 format...
✅ [useSimulator] Parsed 92 memory segments
📍 [useSimulator] Loading segment at 0x000000, size: 15 bytes
📨 [useSimulator] Received message from worker: loadROM
[Worker] Handling message: loadROM
[Worker] LOADROM: Loading 15 bytes
[Worker] LOADROM: Success
✅ [useSimulator] ROM loaded successfully! Total: 1377 bytes
```

---

## What You'll See When Clicking Buttons

### STEP Button
```
🎮 [ControlPanel] STEP button clicked
⏭️ [useSimulator] Stepping one instruction
📨 [useSimulator] Received message from worker: step
[Worker] Handling message: step
[Worker] STEP: Executing one instruction
[Worker] STEP: Done, PC=0x000002
📊 [useSimulator] CPU State updated: {
    pc: "0x000002",
    sr: "0x2700",
    d0: "0x00000000",
    a7: "0x00041fff"
}
```

### PLAY Button
```
🎮 [ControlPanel] PLAY button clicked
▶️ [useSimulator] Running 1000 instructions
📨 [useSimulator] Received message from worker: run
[Worker] Handling message: run
[Worker] RUN: Executing 1000 instructions
[Worker] RUN: Done, PC=0x0000XX
📊 [useSimulator] CPU State updated: {...}
```

### PAUSE Button
```
🎮 [ControlPanel] PAUSE button clicked
⏸️ [useSimulator] Pausing execution
📨 [useSimulator] Received message from worker: pause
[Worker] Handling message: pause
[Worker] PAUSE: Pausing CPU
```

### RESET Button
```
🎮 [ControlPanel] RESET button clicked
🔄 [useSimulator] Resetting CPU
📨 [useSimulator] Received message from worker: reset
[Worker] Handling message: reset
[Worker] RESET: Resetting CPU
[Worker] RESET: Done, PC=0x000000
📊 [useSimulator] CPU State updated: {
    pc: "0x000000",
    sr: "0x2700",
    d0: "0x00000000",
    a7: "0x00041fff"
}
```

---

## Understanding the Log Prefixes

| Prefix | Meaning |
|--------|---------|
| 🚀 | Starting/initialization |
| ✅ | Success |
| ❌ | Error |
| 📨 | Message received |
| 📊 | State update |
| 🔧 | Setup/configuration |
| 🎮 | User interaction |
| ⏭️ | Step instruction |
| ▶️ | Play/run |
| ⏸️ | Pause |
| 🔄 | Reset |
| 🔍 | Parsing/analysis |
| 📍 | Memory location |
| 📥 | File loading |
| 📂 | File operation |
| [Worker] | Messages from Web Worker |
| [useSimulator] | React hook messages |
| [ControlPanel] | UI component messages |

---

## Debugging Tips

### To Filter Logs
In the console, type:
```javascript
// Show only worker logs
$('#worker')

// Show only simulator logs
console.log = function(...args) { if(args[0]?.includes?.('useSimulator')) console.error(...args); }
```

### To Track CPU State Changes
Copy and paste this in the console:
```javascript
// Run after each step and you'll see the full CPU state
JSON.parse(localStorage.getItem('lastCpuState') || '{}')
```

### To See All Messages at Once
```javascript
// Filter for only message receives
console.log = (msg) => { if(msg.includes('Received')) console.error(msg); }
```

---

## Common Issues and How to Debug

### "Simulator not initialized"
- Check for ❌ errors in console during load
- Look for WASM module loading errors
- Refresh the page (hard refresh: Ctrl+Shift+R)

### "STEP not advancing PC"
- Verify the [Worker] logs show "STEP: Done, PC=..."
- Check if CPU state is actually being updated
- Verify ROM is loaded (check "ROM loaded successfully")

### "ROM not loading"
- Look for 📥 [useSimulator] logs
- Check if fetch returns status 200
- Verify parsing shows correct segment count
- Check [Worker] LOADROM logs for success/failure

### "Worker creation failed"
- Check for ❌ errors at the start
- Ensure evm.js and evm.wasm files load (Network tab)
- Hard refresh to clear cache

---

## Performance Monitoring

You can track performance in the console:

```javascript
// Add this to your console for timing
console.time('STEP');
// Click STEP button
console.timeEnd('STEP');
// Will show: "STEP: 0.5ms" or similar
```

---

## Real-Time Monitoring Script

Paste this in the console to monitor automatically:

```javascript
let lastPC = 0;
let stepCount = 0;

setInterval(() => {
    // You can add custom monitoring here
}, 500);

console.log('%c✅ Monitoring active', 'color: green; font-weight: bold');
```

---

## Tips for Troubleshooting

1. **Always start with a hard refresh**: Ctrl+Shift+R
2. **Check Network tab**: Verify evm.wasm and evm.js load correctly
3. **Look for RED errors**: Scroll through console and look for red ❌ lines
4. **Check initialization order**: Should see WASM load before CPU init
5. **Verify ROM loads**: Should see "ROM loaded successfully" before STEP works
6. **Check PC advancement**: Compare PC before and after STEP

---

## Questions to Answer When Debugging

1. Is the worker being created? (Look for ✅ Worker created)
2. Is WASM loading? (Look for [Worker] WASM module loaded)
3. Is CPU initializing? (Look for INIT: CPU initialized)
4. Is ROM loading? (Look for LOADROM: Success)
5. Is PC advancing? (Check PC values in state updates)
6. Are there any red error messages? (❌ or ERROR)

---

That's it! Now open `http://localhost:8084` and check the console. 🚀
