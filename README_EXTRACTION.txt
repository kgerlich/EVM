================================================================================
MC68020 INSTRUCTION EXTRACTION - README
================================================================================

This directory contains extracted MC68020 CPU instruction implementations
from the original EVM simulator (Stcom.c) prepared for WASM conversion.

EXTRACTED DOCUMENTS:
================================================================================

1. EXTRACTION_INDEX.md
   - START HERE: Navigation guide and architecture overview
   - Implementation patterns for each instruction type
   - WASM conversion checklist (Phase 1-6)
   - Carry/overflow algorithm explanations
   - File references and integration guidelines

2. MC68020_INSTRUCTION_EXTRACT.md
   - Complete raw C code for all 9 instructions
   - Helper functions (gen_carry, gen_over)
   - Flag macros and data structures
   - Detailed WASM conversion notes
   - Use for: Full code review, detailed implementation

3. INSTRUCTION_EXTRACTION_SUMMARY.txt
   - Concise summary of each instruction
   - Line numbers and source file references
   - Key implementation details
   - Use for: Quick reference, finding specific information

4. INSTRUCTION_QUICK_REFERENCE.txt
   - Instruction summary table
   - Flag bit encoding documentation
   - Carry/overflow algorithm pseudocode
   - CommandMode dispatch pattern reference
   - Masking patterns and PC advancement rules
   - Use for: During implementation, algorithm reference

EXTRACTED INSTRUCTIONS (9 TOTAL):
================================================================================

Logical Operations (set NEG, ZERO; clear OVER, CARRY):
  1. COM_ori  (Lines 106-170)   - Bitwise OR immediate
  2. COM_andi (Lines 176-229)   - Bitwise AND immediate
  3. COM_eori (Lines 370-423)   - Bitwise XOR immediate

Arithmetic Operations (set all 5 flags):
  4. COM_addi (Lines 303-364)   - Add immediate
  5. COM_subi (Lines 236-297)   - Subtract immediate
  6. COM_addq (Lines 3096-3142) - Add quick (1-8)
  7. COM_subq (Lines 3148-3194) - Subtract quick (1-8)

Compare Operations (set all 5 flags, no write):
  8. COM_cmpi (Lines 429-480)   - Compare immediate
  9. COM_cmpa (Lines 4394-4423) - Compare address register

SUPPORTING CODE EXTRACTED:
================================================================================

Helper Functions:
  - gen_carry(long source, long dest, long result) → char
  - gen_over(long source, long dest, long result) → char

Flag Manipulation Macros:
  - NEG1/NEG0   (Negative flag, bit 3)
  - ZERO1/ZERO0 (Zero flag, bit 2)
  - OVER1/OVER0 (Overflow flag, bit 1)
  - CARRY1/CARRY0 (Carry flag, bit 0)
  - XTEND1/XTEND0 (Extend flag, bit 4)

Data Structures:
  - Opcode union (of) with general and special formats
  - Work structure (work) for source/destination/result
  - CPU state: pc, spc, ccr (condition code), aregs[]

Memory Functions:
  - GETword(unsigned long address) → short
  - GETdword(unsigned long address) → long

SOURCE FILES USED:
================================================================================

Primary:
  /home/kgerlich/dev/EVM/EVMSim/Stcom.c      (5335 lines)
    - Main instruction implementations
    - Opcode union definition
    - CommandMode function pointer array
    - Global CPU state

Supporting:
  /home/kgerlich/dev/EVM/EVMSim/STFLAGS.C    (130 lines)
    - gen_carry() implementation
    - gen_over() implementation
    - Flag setter functions

  /home/kgerlich/dev/EVM/EVMSim/macros.h     (25 lines)
    - Flag manipulation macro definitions

  /home/kgerlich/dev/EVM/EVMSim/STMEM.H      (33 lines)
    - Memory access function declarations

KEY CONCEPTS:
================================================================================

Flag System:
  - 5 condition code flags (N, Z, V, C, X)
  - Stored in cpu.sregs.ccr (8-bit condition code register)
  - Modified via bitwise operations

Size Encoding:
  - 0 = Byte (8-bit, mask 0x000000FF)
  - 1 = Word (16-bit, mask 0x0000FFFF)
  - 2 = Long (32-bit, no mask)

PC Management:
  - cpu.pc: Current program counter (24-bit address)
  - spc: Saved PC (used across CommandMode calls)
  - Typical advancement: opcode (+2), immediate (+2 or +4)

Addressing Modes:
  - CommandMode[8] function pointer array
  - 8 modes: DRD, ARD, ARI, ARIPI, ARIPD, ARID, ARII, MISC
  - Signature: long func(char reg, char cmd, long val, char size)
  - cmd=0 (read), cmd=1 (write)

Carry/Overflow Detection:
  - gen_carry(): Sign-based analysis of source, dest, result
  - gen_over(): Two's-complement overflow detection
  - Used in all arithmetic operations

QUICK START FOR WASM CONVERSION:
================================================================================

Step 1: Understand the Architecture
  Read: EXTRACTION_INDEX.md (sections: Implementation Architecture, Key Concepts)
  Time: 30 minutes

Step 2: Study a Simple Instruction
  Read: MC68020_INSTRUCTION_EXTRACT.md - COM_ori (lines 106-170)
  Read: INSTRUCTION_QUICK_REFERENCE.txt - Logical Operations pattern
  Time: 20 minutes

Step 3: Understand Carry/Overflow
  Read: EXTRACTION_INDEX.md - Carry/Overflow Algorithms section
  Read: INSTRUCTION_QUICK_REFERENCE.txt - Flag Bits in CCR section
  Study: gen_carry() and gen_over() implementations
  Time: 30 minutes

Step 4: Set Up WASM Infrastructure
  Follow: EXTRACTION_INDEX.md - WASM Conversion Checklist, Phase 1
  Implement: CPU state structures, flag macros, memory access stubs
  Time: 2-3 hours

Step 5: Port Instructions
  Follow: EXTRACTION_INDEX.md - WASM Conversion Checklist, Phase 5
  Reference: MC68020_INSTRUCTION_EXTRACT.md for each instruction
  Time: 4-6 hours (all 9 instructions)

Step 6: Test and Verify
  Follow: EXTRACTION_INDEX.md - WASM Conversion Checklist, Phase 6
  Create unit tests for gen_carry/gen_over
  Create tests for each instruction variant
  Time: 3-4 hours

Total Time Estimate: 10-15 hours for complete WASM implementation

VERIFICATION CHECKLIST:
================================================================================

Source Code Extraction:
  [X] All 9 instruction functions extracted with complete C code
  [X] Helper functions gen_carry() and gen_over() extracted
  [X] Flag macros documented
  [X] Data structures identified
  [X] Line numbers verified against source

Documentation:
  [X] Overview document (EXTRACTION_INDEX.md)
  [X] Complete code document (MC68020_INSTRUCTION_EXTRACT.md)
  [X] Summary document (INSTRUCTION_EXTRACTION_SUMMARY.txt)
  [X] Quick reference (INSTRUCTION_QUICK_REFERENCE.txt)
  [X] WASM conversion notes included

Dependencies:
  [X] All required macros extracted
  [X] All required functions identified
  [X] External interfaces documented
  [X] Memory access abstraction defined
  [X] Addressing mode dispatch documented

WASM Readiness:
  [X] All code is position-independent
  [X] No external dependencies (except memory/addressing modes)
  [X] All algorithms documented in pseudocode
  [X] All flag operations clearly explained
  [X] Size handling documented

STATUS: READY FOR WASM CONVERSION

SUPPORT FILES:
================================================================================

Original Source: /home/kgerlich/dev/EVM/EVMSim/
  - Stcom.c (instruction implementations)
  - STFLAGS.C (flag operations)
  - macros.h (macro definitions)
  - STMEM.H (memory interface)

Extracted Documentation: /home/kgerlich/dev/EVM/
  - EXTRACTION_INDEX.md
  - MC68020_INSTRUCTION_EXTRACT.md
  - INSTRUCTION_EXTRACTION_SUMMARY.txt
  - INSTRUCTION_QUICK_REFERENCE.txt
  - README_EXTRACTION.txt (this file)

Related WASM Project:
  - /home/kgerlich/dev/EVM/evm-web/
  - Target: evm-web/evm-core/src/opcodes.c

QUESTIONS & NOTES:
================================================================================

Q: Can I use the extracted code as-is in WASM?
A: Mostly yes, but you'll need to:
   1. Replace bitfield structs with shift/mask operations
   2. Implement CommandMode addressing mode functions
   3. Implement GETword/GETdword with proper endian handling
   4. Port flag macros to JavaScript functions

Q: What's the difference between ORI and ANDI immediate instructions?
A: ORI masks the immediate then ORs with destination
   ANDI masks the immediate AND-mask then ANDs with destination
   This preserves upper bits of destination in ANDI.

Q: Why is spc (saved PC) needed?
A: Because CommandMode() might advance cpu.pc when fetching indirect
   addresses or extensions. We save/restore to ensure the instruction
   handler controls PC advancement.

Q: What do I need to implement for addressing modes?
A: Just the basic stubs for now:
   - DRD (Data Register Direct): Read/write data registers D0-D7
   - ARD (Address Register Direct): Read/write address registers A0-A7
   Other modes can be stubbed for initial WASM implementation.

CONTACT & REFERENCES:
================================================================================

Project: EVM Simulator - MC68020 Emulator for WebAssembly
Repository: /home/kgerlich/dev/EVM/
Guidelines: See CLAUDE.md in EVM directory

Original Author: Klaus P. Gerlicher (1997)
Compiler: Borland C++ 5.0
Extraction Date: November 4, 2025

================================================================================
