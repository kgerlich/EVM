# EVM Simulator Refactoring for WASM

## Current Architecture Problem
- Stcom.c (5300+ lines) mixes pure CPU logic with Windows-specific DLL plugin system
- DLL loading code requires Windows APIs not available in WASM
- Cannot separate platform-specific code easily

## Refactoring Strategy

### Phase 1: Module Separation
Extract from existing code:
- **cpu_core.c**: All COM_* instruction handlers (pure CPU logic, no platform deps)
- **simulator.c**: Core simulation loop, CPU state, memory access (platform-independent)
- **simulator.h**: Public API for simulator

### Phase 2: Replace Plugin System
OLD (Windows DLL-based):
- LoadLibrary() to load .DLL files
- GetProcAddress() to get function pointers
- FindFirstFile/FindNextFile to enumerate plugins

NEW (Static initialization):
- Initialize RAM, ROM, 68230, 68681 directly in code
- Call setup/init functions directly
- No dynamic loading needed

### Phase 3: Platform Abstraction
Create thin abstraction layers:
- Memory allocation (malloc/free)
- Configuration loading (stub in WASM)
- UI updates (callback-based or noop)
- Threading (simulator.c has no threads, just execution loop)

### Phase 4: WASM Bindings
- wasm/bindings.c calls simulator.h API
- No Windows calls, pure C99 code

## Files to Create
- `src/cpu_core.c` - CPU instruction handlers (extracted)
- `src/simulator.c` - Core simulator
- `src/simulator_init.c` - Platform-independent peripheral initialization
- `include/simulator.h` - Public API

## Build Changes
- CMakeLists.txt uses modular files instead of monolithic Stcom.c
- Windows build still uses Stcom.c + stdll.h plugin system
- WASM build uses new modular structure (no DLLs)
