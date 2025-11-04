# EVM Web Frontend

A modern web-based interface for the MC68020 CPU simulator, built with React + TypeScript and WebAssembly.

## Features

- **🚀 WebAssembly-Based**: Compiles the C simulator directly to WASM for native-speed execution
- **⚛️ React UI**: Modern, responsive interface with real-time visualization
- **🎮 Interactive Controls**: Play, pause, step, and reset the simulator
- **💾 Memory Inspector**: Browse and edit memory at any address
- **📊 CPU Register Display**: Live updates of all registers and flags
- **💬 Terminal Emulator**: Serial I/O from the 68681 UART
- **🔍 Disassembler**: View instructions with opcodes and cycle counts
- **🖲️ Peripheral Monitor**: Real-time register display for PIT and UART
- **📦 Offline Capable**: Everything runs in the browser, no server needed

## Project Structure

```
evm-web/
├── evm-core/               # C code + Emscripten build
│   ├── CMakeLists.txt      # Build configuration
│   ├── wasm/
│   │   └── bindings.c      # WASM/JS interface
│   └── build.sh            # Build script
│
├── web/                    # React frontend (Vite)
│   ├── src/
│   │   ├── components/     # React components
│   │   ├── hooks/          # useSimulator hook
│   │   ├── workers/        # Web Worker
│   │   ├── App.tsx         # Main application
│   │   └── main.tsx        # Entry point
│   ├── public/             # Static assets
│   └── vite.config.ts      # Vite configuration
│
└── README.md               # This file
```

## Getting Started

### Prerequisites

- **Node.js** v18+ (for npm and development)
- **Emscripten** SDK (for WASM compilation)
- **CMake** 3.15+ (for build configuration)

### Installation

```bash
# Install Emscripten (if not already installed)
# https://emscripten.org/docs/getting_started/downloads.html
emsdk install latest
emsdk activate latest
source emsdk_env.sh

# Install Node dependencies
cd evm-web/web
npm install
```

### Development

**1. Build the WASM module:**

```bash
cd evm-core
./build.sh
```

This generates:
- `../web/public/evm.js` - WASM loader
- `../web/public/evm.wasm` - Binary module

**2. Run the development server:**

```bash
cd web
npm run dev
```

Opens at `http://localhost:5173`

**3. Build for production:**

```bash
# Build WASM
cd evm-core
./build.sh

# Build React app
cd ../web
npm run build
```

Output in `web/dist/`

## Architecture

### WASM Bindings (evm-core/wasm/bindings.c)

Exported C functions available to JavaScript:

- `cpu_init()` - Initialize simulator
- `cpu_reset()` - Reset to initial state
- `cpu_step()` - Execute one instruction
- `cpu_run(count)` - Execute N instructions
- `cpu_pause()` - Pause execution
- `cpu_get_state()` - Retrieve CPU state
- `cpu_read_byte/word/dword(addr)` - Memory reads
- `cpu_write_byte/word/dword(addr, val)` - Memory writes
- `cpu_load_rom(data)` - Load ROM image
- `cpu_load_program(data, addr)` - Load program into RAM

### Web Worker (src/workers/simulator.worker.ts)

Runs the simulator in a background thread, preventing UI blocking:

- Loads and initializes WASM module
- Handles CPU execution commands
- Returns CPU state updates
- Thread-safe memory access

### React Hook (src/hooks/useSimulator.ts)

Provides a clean interface to the simulator:

```typescript
const simulator = useSimulator();

// Execute simulator
simulator.step();
simulator.run(10000);
simulator.pause();
simulator.reset();

// Memory operations
const data = await simulator.readMemory(0x400000, 256);
await simulator.writeMemory(0x400000, data);

// File loading
await simulator.loadROM(romData);
await simulator.loadProgram(programData, 0x400000);
```

### Components

- **CPURegisters** - Shows D0-D7, A0-A7, PC, SR, stack pointers
- **MemoryInspector** - Browse memory with hex dump and ASCII view
- **Terminal** - VT100 serial I/O emulator
- **Disassembler** - Shows next 10 instructions with mnemonics
- **PeripheralMonitor** - PIT and UART register displays
- **ControlPanel** - Play/Pause/Step/Reset, file loading, speed control

## Building WASM

The build system uses **CMake + Emscripten**:

```bash
cd evm-core/build
emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Key CMake options:
- `ALLOW_MEMORY_GROWTH=1` - Allows dynamic memory expansion
- `-O2` - Optimize for speed
- `-Oz` - Optimize for size

Generated files:
- `evm.js` - ~500KB - JavaScript wrapper and loader
- `evm.wasm` - ~1.5MB - Binary WebAssembly module

## Performance

**Execution Speed:**
- ~100,000 instructions/second in WASM
- Modern browsers (Chrome, Firefox, Safari) all perform similarly

**Memory Usage:**
- ~16MB virtual address space (24-bit addressing)
- ~2-3MB browser memory for WASM module

**Optimization Tips:**
1. Adjust execution speed slider for balance between responsiveness and throughput
2. Use Step mode for debugging single instructions
3. Use Run mode for faster program execution

## Deployment

### Vercel (Recommended)

```bash
vercel deploy evm-web/web
```

### GitHub Pages

```bash
cd web
npm run build
# Commit dist/ folder
```

### Self-Hosted

```bash
# Build and serve
npm run build
npx serve dist
```

**Important Headers for Web Workers:**
Your server must set these CORS headers for SharedArrayBuffer support:

```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

## Troubleshooting

### WASM module not loading

- Verify `public/evm.js` and `public/evm.wasm` exist
- Check browser console for CORS errors
- Ensure MIME type for `.wasm` is `application/wasm`

### Simulator not responding

- Check if Web Worker initialized successfully
- Look at browser console for errors
- Try resetting the simulator

### Slow performance

- Lower execution speed slider
- Check CPU usage in Task Manager/Activity Monitor
- Try a different browser

### Memory address errors

- Ensure addresses are 24-bit (0x000000 - 0xFFFFFF)
- ROM is at 0x000000-0x00FFFF (read-only)
- RAM is at 0x400000-0x41FFFF

## Development Notes

### Adding New Instructions

1. Add opcode definition to `script.txt` in EVMSim/
2. Implement handler in `Stcom.c`
3. Rebuild WASM: `./build.sh` in evm-core/
4. Update disassembler table in `Disassembler.tsx` if needed

### Extending Components

All components are self-contained React modules:

```typescript
// Example: Add new component
import React from 'react';
import styles from './MyComponent.module.css';

export const MyComponent: React.FC<Props> = (props) => {
    return <div className={styles.container}>...</div>;
};
```

### Testing the Simulator

```bash
# Manual test with sample S19 file
# 1. Open web app
# 2. Click "Load ROM"
# 3. Select PS20.S19 from evm-web/evm-core/../
# 4. Click Play
```

## Resources

- **68K Instruction Set**: [Motorola 68000 Programmer's Reference](http://ww1.microchip.com/downloads/en/DeviceDoc/M68000PRM.pdf)
- **WebAssembly**: [MDN WebAssembly Documentation](https://developer.mozilla.org/en-US/docs/WebAssembly)
- **React**: [React Documentation](https://react.dev)
- **Emscripten**: [Emscripten Documentation](https://emscripten.org/docs)

## License

MIT License - See LICENSE file in repository root

## Contributing

Contributions welcome! Please:
1. Test WASM compilation
2. Test React components in browser
3. Update documentation
4. Follow existing code style
