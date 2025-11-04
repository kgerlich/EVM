# CPU Instruction Implementation - Complete Reference Index

## Overview

This index provides a complete guide to understanding and implementing MC68020 CPU instructions in the EVM WebAssembly simulator.

## Documentation Files

### 1. CPU_INSTRUCTION_ANALYSIS.md
**Comprehensive technical reference** (570 lines, 19KB)

Answers all 5 key questions:
1. What CPU instruction handlers are already implemented?
2. What is the structure/pattern for implementing a CPU instruction?
3. Where is opcode dispatch/lookup happening?
4. What opcodes are NOT yet implemented? Is JSR implemented?
5. Are there stub/placeholder instruction handlers I can use as template?

**Includes:**
- 24 fully implemented instruction list with opcodes
- Instruction implementation pattern with code templates
- Opcode dispatch mechanism (65,536-entry jump table)
- JSR/JMP status and other unimplemented opcodes
- 5 instruction complexity templates (simple to complex)
- Key implementation details and file organization

**Location:** `/home/kgerlich/dev/EVM/CPU_INSTRUCTION_ANALYSIS.md`

### 2. CPU_INSTRUCTION_QUICK_REFERENCE.md
**Quick lookup guide** (308 lines, 8.6KB)

Quick reference for common tasks:
- Key files directory
- Opcode dispatch flow diagram
- Implemented instructions checklist
- Instruction implementation checklist
- Code templates for 5 complexity levels
- Common mistakes and fixes
- Example: Implementing ADDI

**Location:** `/home/kgerlich/dev/EVM/CPU_INSTRUCTION_QUICK_REFERENCE.md`

## Key Source Files (with absolute paths)

### Core Instruction Implementation
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c`
  - Lines 40-1450: 24 instruction handlers
  - Lines 19-35: Opcode union 'of' definition
  - COM_add, COM_jsr, COM_jmp, COM_bra, COM_nop, etc.

### Opcode Dispatch Table
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/sttable.c`
  - Lines 1-50: Extern declarations
  - Lines 95-65630: 65,536-entry Operation[] jump table
  - Lines 20193-20239: JSR/JMP opcode mappings

### Main Simulation Loop
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions_wrapper.c`
  - Lines 5265-5310: Simulate68k() function
  - Opcode fetch, dispatch, and interrupt handling

### Data Structures & Headers
- `/home/kgerlich/dev/EVM/evm-web/evm-core/include/STSTDDEF.H`
  - Lines 35-47: CPU struct definition
  - Lines 50-54: tag_work struct
  - Line 84: CACHEFUNCTION macro

- `/home/kgerlich/dev/EVM/evm-web/evm-core/include/STMEM.H`
  - Lines 18-31: Memory access functions

- `/home/kgerlich/dev/EVM/evm-web/evm-core/include/STFLAGS.H`
  - Lines 13-50: Flag manipulation functions
  - Lines 54-63: Flag macros (NEG0/1, ZERO0/1, etc.)

### Stub Templates
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/instruction_stubs.c`
  - Lines 31-36: STUB_HANDLER macro
  - Lines 39-127: 70+ stub implementations

## Quick Navigation

### Find an Instruction Implementation
Use grep pattern: `grep -n "^void COM_<name>" cpu_instructions.c`

Examples:
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c:40` - COM_add
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c:406` - COM_jsr
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c:371` - COM_jmp
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c:250` - COM_bra
- `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c:933` - COM_nop

### Understand Opcode Dispatch
1. Start at: `cpu_instructions_wrapper.c:5265` (Simulate68k function)
2. See: `of.o = GETword(cpu.pc);` (fetch opcode)
3. See: `Operation[of.o](of.o);` (dispatch to handler)
4. Find table at: `sttable.c:95` (Operation[] array definition)
5. Find mapping at: `sttable.c:20193+` (opcode ranges)

### Implement New Instruction
1. Copy template from Quick Reference or Analysis docs
2. Implement handler in `cpu_instructions.c`
3. Add extern in `sttable.c` (~line 10)
4. Map opcode(s) in `Operation[]` array
5. Rebuild: `cd evm-web/evm-core && ./build.sh`
6. Test with assembly code

## Key Concepts

### Opcode Decoding Union
```c
union {
    unsigned short o;              // Raw 16-bit opcode
    struct {
        unsigned regsrc:3;         // Bits 2-0
        unsigned modesrc:3;        // Bits 5-3
        unsigned modedest:3;       // Bits 8-6
        unsigned regdest:3;        // Bits 11-9
        unsigned group:4;          // Bits 15-12
    } general;
} of;
```

### Standard Handler Pattern
```c
void COM_<name>(short opcode)
{
    CACHEFUNCTION(COM_<name>);     // Optimization hint
    cpu.pc += 2;                   // Skip opcode
    
    switch(of.general.modedest)    // Decode destination
    {
        case 0:                    // Byte
            // ... implementation ...
            if(...) ZERO1; else ZERO0;
            if(...) NEG1; else NEG0;
            break;
        // ... more cases
    }
}
```

### Memory Access
```c
char GETbyte(unsigned long addr);          // Read 8-bit
short GETword(unsigned long addr);         // Read 16-bit
long GETdword(unsigned long addr);         // Read 32-bit
void PUTbyte(unsigned long addr, char);    // Write 8-bit
void PUTword(unsigned long addr, short);   // Write 16-bit
void PUTdword(unsigned long addr, long);   // Write 32-bit
```

### Flag Macros
```c
NEG0/NEG1      // Negative flag (bit 3 of SR)
ZERO0/ZERO1    // Zero flag (bit 2)
CARRY0/CARRY1  // Carry flag (bit 0)
OVER0/OVER1    // Overflow flag (bit 1)
XTEND0/XTEND1  // Extend flag (bit 4)
```

## Instruction Status Summary

### Fully Implemented (24)
ADD, AND, BEQ, BNE, BRA, CLR, CMP, EOR, JMP, JSR, LEA, MOVEFROMSR, MOVEQUICK, MOVETOSR, NEG, NOP, NOT, OR, PEA, RESET, RTE, RTS, SUB, TST

### Stubs (70+)
MUL, MULU, MULS, DIV, DIVX, ABCD, SBCD, PACK, UNPACK, BITFIELD, LINEA, LINEF, and others

### Total Coverage
- Fully Working: 24 instructions (~0.04% of 65,536 opcodes)
- Stubs: 70+ instructions
- Total: 65,536 opcode table entries

## Development Workflow

### To Add a New Instruction

1. **Design**: Choose instruction name (e.g., ADDI)
2. **Implement**: Add handler function in cpu_instructions.c
3. **Register**: Add extern declaration in sttable.c
4. **Map**: Add opcode(s) to Operation[] array
5. **Build**: `cd evm-web/evm-core && ./build.sh`
6. **Test**: Create assembly test program with instruction
7. **Verify**: Check register/memory state in simulator

### Files to Modify

- **cpu_instructions.c**: Add handler function
- **sttable.c**: Add extern declaration and Operation[] entries

### Build Command
```bash
cd /home/kgerlich/dev/EVM/evm-web/evm-core && ./build.sh
```

## Common Patterns

### Simple (NOP-style)
Just increment PC, no flags. See `COM_nop` at line 933.

### With Flags
Read operand, update flags. See `COM_tst` at line 1395.

### Arithmetic (multiple sizes)
Switch on modedest for byte/word/long variants. See `COM_add` at line 40.

### Branch
Decode displacement from opcode. See `COM_bra` at line 250.

### Subroutine Call
Save return address on stack. See `COM_jsr` at line 406.

## Related Documentation

Project CLAUDE.md: `/home/kgerlich/dev/EVM/CLAUDE.md`
- EVM project overview
- Build system details
- Web frontend information
- Plugin architecture

## Questions & Answers

**Q: Is JSR implemented?**
A: Yes, fully implemented at cpu_instructions.c:406-504. Supports modes 2, 5, 6.

**Q: How do I find where an instruction is implemented?**
A: Grep for `void COM_<name>` in cpu_instructions.c

**Q: What's the opcode for instruction X?**
A: Check sttable.c comments (e.g., `// $4E90` for JSR)

**Q: How do I implement a new instruction?**
A: Follow the 5-step workflow in "Development Workflow" section

**Q: Where is the main simulation loop?**
A: cpu_instructions_wrapper.c:5265 (Simulate68k function)

**Q: How is opcode dispatch implemented?**
A: Direct array indexing: `Operation[of.o](of.o)` where Operation[] is the 65,536-entry jump table in sttable.c:95

## Version Information

- **MC68020 CPU**: 32-bit Motorola processor
- **Addressing Modes**: 8 modes (Dn, An, (An), (An)+, -(An), (d,An), (d,An,Xn), Special)
- **Opcode Space**: 65,536 possible opcodes (16-bit)
- **Current Implementation**: ~0.04% complete, focus on essential instructions

---

Created: 2025-11-04
Last Updated: 2025-11-04
Status: Complete analysis of instruction implementation architecture
