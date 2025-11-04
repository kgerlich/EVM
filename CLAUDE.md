# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**EVM (Embedded Virtual Machine) Simulator** - A complete MC68020 CPU emulator for Windows (32-bit) created as a diploma project in 1997 using Borland C++ 5.0. The simulator allows execution of MC68020 machine code with emulated peripherals (timer, serial ports, GPIO) in a Windows GUI environment.

A modern **React + WebAssembly web frontend** has been added, allowing the simulator to run in browsers with performance comparable to the native Windows version.

## Common Development Tasks

### Building the Native Simulator (Windows/MSVC)

The project uses MSVC 6.0 workspace format (.dsw files):

```bash
# Regenerate opcode jump table (required after modifying script.txt)
.\build.exe

# Build entire workspace from command line (if MSVC installed)
msvc evm.dsw  # Or use Visual Studio IDE
```

**Important**: The `build.exe` tool generates `EVMSim/sttable.c` (5000+ lines) from `script.txt`. This file must be regenerated when adding new CPU instructions or modifying opcode definitions.

### Building the Web Frontend

**Prerequisites:**
- Emscripten SDK (https://emscripten.org/docs/getting_started/downloads.html)
- Node.js v18+
- CMake 3.15+

**Build steps:**

```bash
# 1. Build WebAssembly module
cd evm-web/evm-core
./build.sh

# 2. Build React application
cd ../web
npm install        # First time only
npm run build      # Production build
```

**Development workflow:**

```bash
# Start local development server on 0.0.0.0:8084 (auto-recompiles on changes)
cd evm-web/web
npm run dev -- --host 0.0.0.0 --port 8084

# Type check & lint while developing
npm run lint
```

### Running the Simulator

**Web version (development):**
```bash
cd evm-web/web
npm run dev -- --host 0.0.0.0 --port 8084
# Access at http://0.0.0.0:8084
```

**Web version (production):**
```bash
cd evm-web/web
npm run build                                    # Build first
npm run preview -- --host 0.0.0.0 --port 8084  # Run on http://0.0.0.0:8084
```

**Native Windows version:**
```bash
.\EVMSim\EVMSIM.exe
```

The simulator displays:
- Main control window (Start/Stop buttons, status display)
- CPU registers (D0-D7, A0-A7, PC, SR, flags)
- Memory inspector (hex dump viewer)
- 68230 PIT register viewer dialog
- 68681 UART VT100 terminal emulator
- Disassembler (next 10 instructions)

### Debugging

The project includes a `debug.dll` module that can be loaded for debugging support. Trace mode can be enabled via the SR (status register) TRACE bit to trigger single-step execution.

**Web frontend debugging:**
- Use browser DevTools (F12) to inspect React components and Web Worker
- Check console for simulator errors
- Use the Memory Inspector to view/edit memory at runtime

## Architecture Overview

### Plugin-Based Modular Design

The simulator uses a **dynamic DLL plugin architecture** with a standardized interface:

```
EVMSIM.EXE (Main Application)
    ├─ CPU Core (Stcom.c) - Opcode execution engine
    ├─ Memory Manager (Stmem.c) - Address mapping
    ├─ Exception Handler (Stexep.c) - Interrupts & exceptions
    └─ Plugin Loader
        ├─ EVMROM.DLL (64KB ROM memory at 0x000000)
        ├─ EVMRAM.DLL (128KB RAM at 0x400000)
        ├─ 68230.DLL (MC68230 PIT at 0x800000)
        └─ 68681.DLL (MC68681 DUART at 0xA00000)
```

Each DLL plugin exports a `DLLHDR` structure (defined in `stdll.h`) with function pointers for: Setup, Exit, Reset, Simulate, Read, Write, and configuration (address, size, priority, cacheable flag).

### Memory Map

| Address Range | Module | Size | Purpose |
|---|---|---|---|
| 0x000000-0x00FFFF | EVMROM | 64 KB | System ROM (loaded from S19 file) |
| 0x400000-0x41FFFF | EVMRAM | 128 KB | RAM |
| 0x800000-0x800035 | 68230 PIT | 54 bytes | Parallel I/O & Timer |
| 0xA00000-0xA0001F | 68681 UART | 32 bytes | Serial interface (2 channels) |

Address resolution uses 10-bit indexed lookup tables (16K entries) mapping to plugin Read/Write function pointers. Unmapped addresses trigger BUS ERROR exceptions.

### CPU Core Execution Model

**Main Execution Loop** (`Simulate68k()` in Stcom.c:5243):

1. Check for address errors (uneven PC)
2. Fetch opcode from memory
3. Set up stack pointer based on CPU privilege mode
4. Save PC for exception handling
5. Decode and execute via `Operation[]` jump table
6. Call `SimProc()` on all loaded plugins (peripheral simulation)
7. Check for pending interrupts

The simulator executes ~100,000 MC68020 instructions per second. The `Operation[]` array is a 65,536-entry jump table to instruction handlers (e.g., `COM_move()`, `COM_add()`, `COM_ori()`).

### Key Components

| Component | File | Lines | Purpose |
|---|---|---|---|
| CPU Execution | Stcom.c | 8000+ | Opcode handlers, address mode calculations, instruction logic |
| Memory Manager | Stmem.c | 2000+ | Address mapping, device routing, endian conversion |
| Exception Handler | Stexep.c | 1000+ | Interrupts, exceptions, privilege checking |
| Windows UI | Stmain.c | 1000+ | GUI, message loop, simulation thread management |
| Plugin Interface | stdll.h | 100+ | Standard DLL plugin interface definition |
| Opcode Table | sttable.c | 5000+ | **Generated** - Jump table created by build.exe |

### Plugin System

To add a new peripheral device:

1. Create a new DLL project
2. Implement required functions:
   - `SetupProc()` - Initialize hardware state
   - `ExitProc()` - Cleanup
   - `ResetProc()` - Reset to known state
   - `SimProc()` - Called once per opcode (handles timer ticks, state updates)
   - `ReadProc()` - Handle CPU reads from device address space
   - `WriteProc()` - Handle CPU writes to device address space
3. Export a `DLLHDR` structure using the `MAKE_HDR` macro
4. Create a `.INI` configuration file with device settings
5. Place the DLL in the plugin directory (specified in `EVMSIM.INI`)

The plugin interface automatically handles CPU interrupts via the `CONN_TO_CPU` structure.

### CPU Architecture Details

**Register Set:**
- 8 Data registers (D0-D7): 32-bit
- 8 Address registers (A0-A7): 32-bit, A7 is stack pointer
- Program Counter (PC): 24-bit address space
- Status Register (SR): Includes IPL (interrupt priority level), flags (N, Z, V, C), and TRACE bit
- Control registers: SFC, DFC (source/destination FC), VBR (vector base), CACR, CAAR

**Exception System:**
- Bus Error, Address Error, Illegal Opcode, Privilege Violation, Division by Zero, Trace, Line A/F
- Interrupt priorities: IPL 0-7 with privilege level checking
- Proper 68020 exception frame formatting on stack

**Addressing Modes** (8 total, implemented in Steacalc.c):
1. Data Register Direct (Dn)
2. Address Register Direct (An)
3. Address Register Indirect (@An)
4. Indirect with Post-increment (@An+)
5. Indirect with Pre-decrement (-(An))
6. Indirect with Displacement (@(d16,An))
7. Indirect with Index (@(d8,An,Xn))
8. Special (PC-relative, absolute, immediate)

### Threading Model

- **Main GUI Thread**: Windows message loop, timers, menu handling
- **Simulation Thread**: Continuous opcode execution (~100K ops per scheduler yield)
- **Timer Thread** (68230): Generates hardware timer interrupts every 1ms

Synchronization: Global `bRunning` flag controls simulation lifetime. Plugin `SimProc()` calls are synchronous (one per opcode).

## Adding New CPU Instructions

1. **Define the instruction** in `script.txt`:
   - Format: `OPCODE_RANGE CYCLES MASK FUNCTION_NAME`
   - Example: `0xD000-0xDFFF 6 0xFFC0 COM_add`

2. **Run build.exe** to regenerate `sttable.c`:
   ```bash
   .\build.exe
   ```

3. **Implement the handler** in `Stcom.c`:
   ```c
   void COM_add(unsigned short opcode)
   {
       // Decode operands using of (opcode union)
       // Perform operation
       // Set flags (N, Z, V, C) in cpu.sr
       // Handle exceptions if needed
   }
   ```

4. **Key macros** in Stcom.c:
   - `GETbyte/word/dword()` - Read from memory
   - `PUTbyte/word/dword()` - Write to memory
   - `GET_EA_*()` - Calculate effective addresses
   - `CALC_FLAGS()` - Set condition codes

## File Organization

```
/e/dev/evm/
├── EVMSim/           # Main application
│   ├── Stcom.c       # CPU core (most instruction logic here)
│   ├── Stmain.c      # Windows UI and threading
│   ├── Stmem.c       # Memory addressing
│   ├── Stexep.c      # Exceptions/interrupts
│   ├── Steacalc.c    # Address mode calculations
│   ├── sttable.c     # GENERATED opcode jump table
│   ├── script.txt    # Opcode definitions (input to build.exe)
│   └── *.h           # Header files
├── 68230/            # MC68230 PIT peripheral DLL
├── 68681/            # MC68681 UART peripheral DLL
├── evmrom/           # ROM DLL
├── evmram/           # RAM DLL
├── build/            # Code generator tool
├── VTune/            # Performance profiling files
├── evm.dsw           # MSVC workspace
└── *.INI             # Configuration files
```

## Important Implementation Notes

### Endian Handling

The MC68020 uses **big-endian** byte order while x86 is **little-endian**. Conversion occurs in all memory read/write functions. Watch for:
- `swap_word()` and `swap_dword()` macros
- Multi-byte value handling in `GETword()`, `PUTword()`, etc.

### Address Space

- 24-bit address space (0x000000 - 0xFFFFFF)
- Addresses are masked with `0x00FFFFFF` throughout
- Odd (uneven) PC addresses trigger address error exceptions

### Register Naming

Watch for different naming conventions:
- `cpu.aregs.a[7]` - Stack pointer (A7)
- `cpu.sregs.sr` - Status register
- `cpu.pc` - Program counter
- `cpu.ssp`, `cpu.usp`, `cpu.msp` - Supervisor, user, master stack pointers

### Opcode Union

Instructions are decoded using the `of` (opcode) union structure to extract fields by bit position (bitfields). This allows efficient decoding of different instruction formats.

## Configuration Files

Each module can be configured via `.INI` files:

```ini
; EVMSIM.INI - Main application config
[EVMSIM]
PLUGINDIR=.\

; 68230.INI - PIT configuration
[HARDWARE]
IPL=3

; EVMROM.INI - ROM configuration
SYSTEMFILE=PS20.S19
```

## Performance Considerations

- Opcode execution: ~100,000 instructions/second
- Memory access via lookup tables (fast)
- Plugin `SimProc()` called once per opcode (synchronous, can bottleneck)
- Attempted opcode caching (CACHEFUNCTION macro, not fully implemented)
- Status bar shows ops/sec and cache statistics

## Development Environment

- **Original Compiler**: Borland C++ 5.0
- **Target Environment**: Windows 95/98/NT (32-bit)
- **Modern Compatibility**: Can be compiled with MSVC 6.0+
- **Build System**: MSVC workspace (.dsw, .dsp files)
- **Dependencies**: Windows API (Win32), no external libraries

## Web Frontend

A modern React + WebAssembly frontend is available in the `evm-web/` directory.

### Web Frontend Stack

- **WASM**: Compiled C simulator via Emscripten (~100K instructions/sec, 1.5MB binary)
- **React + TypeScript**: Modern type-safe components (React 19, TypeScript 5.9)
- **Vite**: Fast development server and production bundler
- **Web Worker**: Simulator runs in background thread (prevents UI blocking)
- **Responsive UI**: Works on desktop, tablet, mobile

### Development Commands

```bash
cd evm-web/web

# Start development server on 0.0.0.0:8084 (auto-recompile on changes)
npm run dev -- --host 0.0.0.0 --port 8084
# Access at http://0.0.0.0:8084

# Type checking & linting
npm run lint                # Run ESLint
npm run build              # Full TypeScript build + production bundle

# Preview production build locally (on port 8084)
npm run preview -- --host 0.0.0.0 --port 8084
# Access at http://0.0.0.0:8084
```

### Web Components

- **Control Panel** - Play, pause, step, reset with speed control
- **CPU Registers** - Live view of D0-D7, A0-A7, PC, SR with flags
- **Memory Inspector** - Hex dump viewer (256 bytes at a time)
- **Disassembler** - Shows next 10 instructions with mnemonics
- **Terminal Emulator** - VT100 serial I/O (68681 UART)
- **Peripheral Monitor** - MC68230 and MC68681 register display

### Building WASM Module

The WASM module is compiled from the C simulator using Emscripten:

```bash
cd evm-web/evm-core
./build.sh

# Output:
#   ../web/public/evm.js    (~500KB)
#   ../web/public/evm.wasm  (~1.5MB)
```

The build script:
1. Checks for Emscripten (emcc command)
2. Runs CMake with Emscripten toolchain
3. Compiles C code to WASM with optimization flags (-O2)
4. Copies output to web/public/

**Important**: Rebuild WASM after modifying C code in `EVMSim/` or when adding new CPU instructions.

### WASM Bindings

Key C functions exported to JavaScript from `evm-core/wasm/bindings.c`:
- `cpu_init()` / `cpu_reset()` - Initialize/reset simulator
- `cpu_step()` / `cpu_run(count)` / `cpu_pause()` - Execution control
- `cpu_get_state()` - Retrieve full CPU state
- `cpu_read_byte/word/dword(addr)` - Memory reads
- `cpu_write_byte/word/dword(addr, val)` - Memory writes
- `cpu_load_rom(data)` / `cpu_load_program(data, addr)` - Load code

See `evm-core/CMakeLists.txt` for compilation flags and `web/src/workers/simulator.worker.ts` for Web Worker integration.

### Production Build

```bash
cd evm-web/web

# Full build (TypeScript check + WASM + React bundle)
npm run build

# Output: web/dist/
#   - index.html
#   - *.js, *.css (React app)
#   - evm.js, evm.wasm (WASM module)
```

**Note**: WASM files must be pre-built in `web/public/` before running React build.

### Deployment

Deploy `evm-web/web/dist/` to any static host:
- **Vercel (recommended)**: `vercel deploy`
- **GitHub Pages**: Push to gh-pages branch
- **Netlify/AWS S3**: Standard static hosting

**Required server headers for Web Worker support:**
```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

See `evm-web/README.md` for additional details.
