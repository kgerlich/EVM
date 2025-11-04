# MC68020 Instruction Handler Extraction - Complete Index

**Date:** November 4, 2025  
**Source:** `/home/kgerlich/dev/EVM/EVMSim/Stcom.c` (5335 lines)  
**Extraction Status:** Complete - 9 of 9 instructions extracted with all dependencies

---

## Quick Navigation

### Extracted Documents

1. **MC68020_INSTRUCTION_EXTRACT.md** (21 KB)
   - Complete C code for all 9 instruction handlers
   - Helper functions (gen_carry, gen_over)
   - Flag manipulation macros
   - Data structures and globals
   - Detailed notes for WASM conversion
   - **USE THIS FOR:** Full code review, architecture understanding, WASM porting

2. **INSTRUCTION_EXTRACTION_SUMMARY.txt** (5 KB)
   - Concise summary of each instruction
   - Line numbers and file references
   - Pattern documentation
   - Helper function descriptions
   - **USE THIS FOR:** Quick lookup, finding specific information, references

3. **INSTRUCTION_QUICK_REFERENCE.txt** (8 KB)
   - Table format summary
   - Flag bit encoding
   - Carry/overflow algorithms (plain English)
   - CommandMode dispatch patterns
   - WASM conversion checklist
   - **USE THIS FOR:** Implementation guide, WASM porting, algorithm reference

4. **EXTRACTION_INDEX.md** (This file)
   - Navigation and file organization
   - Key algorithms explained
   - WASM compatibility notes
   - Integration guidelines

---

## Instruction Summary Table

| # | Name    | Type    | Sizes | Flags | Line  | Status |
|---|---------|---------|-------|-------|-------|--------|
| 1 | COM_ori | OR Imm  | B,W,L | N,Z   | 106   | Ready  |
| 2 | COM_andi| AND Imm | B,W,L | N,Z   | 176   | Ready  |
| 3 | COM_subi| SUB Imm | B,W,L | All   | 236   | Ready  |
| 4 | COM_addi| ADD Imm | B,W,L | All   | 303   | Ready  |
| 5 | COM_eori| XOR Imm | B,W,L | N,Z   | 370   | Ready  |
| 6 | COM_cmpi| CMP Imm | B,W,L | All   | 429   | Ready  |
| 7 | COM_addq| ADD Q   | B,W,L | All   | 3096  | Ready  |
| 8 | COM_subq| SUB Q   | B,W,L | All   | 3148  | Ready  |
| 9 | COM_cmpa| CMP Addr| W,L   | All   | 4394  | Ready  |

---

## Dependencies Summary

### Core Global State

```c
union {unsigned short o; ...} of;           // Opcode fields
struct {long source, destination, result;} work;  // Work registers
long cpu.pc;                                 // Program counter
long spc;                                    // Saved PC
struct {unsigned char ccr;} cpu.sregs;      // Condition Code Register
unsigned char cpu.aregs.a[8];               // Address registers (for CMPA)
```

### Flag Manipulation

**Macros (from macros.h):**
- `NEG1/NEG0`, `ZERO1/ZERO0`, `OVER1/OVER0`, `CARRY1/CARRY0`, `XTEND1/XTEND0`
- All operate on `cpu.sregs.ccr` via bitwise AND/OR

**Functions (from STFLAGS.C):**
- `gen_carry(long s, long d, long r)` → char (0/1)
- `gen_over(long s, long d, long r)` → char (0/1)
- `setcarry(char flag)`, `setover(char flag)`, `setxtend(char flag)`

### Memory Access

**Functions (from STMEM.C):**
- `GETword(unsigned long address)` → short (big-endian)
- `GETdword(unsigned long address)` → long (big-endian)

### Addressing Mode Dispatch

**Function pointer array (Stcom.c:89-90):**
```c
long (*CommandMode[8])(char reg, char cmd, long val, char size)
```

Modes: DRD, ARD, ARI, ARIPI, ARIPD, ARID, ARII, MISC

---

## Implementation Architecture

### Pattern 1: Logical Operations (ORI, ANDI, EORI)

```
Loop over sizes (0=byte, 1=word, 2=long):
  1. Advance PC by 2
  2. Read immediate value (GETword for byte/word, GETdword for long)
  3. Advance PC by +2 (word/byte) or +4 (long)
  4. Read destination via CommandMode (cmd=0)
  5. Perform bitwise operation (masked by size)
  6. Write result back via CommandMode (cmd=1)
  7. Set NEG, ZERO flags based on result
  8. Clear OVER, CARRY
```

### Pattern 2: Arithmetic Operations (ADDI, SUBI)

```
Loop over sizes:
  1. Advance PC by 2
  2. Read immediate value
  3. Advance PC by +2 (word/byte) or +4 (long)
  4. Read destination via CommandMode (cmd=0)
  5. Perform add/subtract
  6. Write result via CommandMode (cmd=1)
  7. Set all 5 flags using gen_carry() and gen_over()
  8. Set NEG, ZERO flags
```

### Pattern 3: Compare Operations (CMPI, CMPA)

```
For CMPI:
  Loop over sizes:
    1. Advance PC by 2
    2. Read immediate value
    3. Advance PC by +2 or +4
    4. Read destination via CommandMode (cmd=0)
    5. Subtract: result = destination - source
    6. Set all 5 flags (no write back)

For CMPA:
    1. Advance PC by 2
    2. Decode modedest field (3=word, 7=long)
    3. Read source via CommandMode
    4. Subtract from cpu.aregs.a[regdest]
    5. Set all 5 flags (no register write)
```

### Pattern 4: Quick Operations (ADDQ, SUBQ)

```
1. Advance PC by 2
2. Extract size: (opcode >> 6) & 0x03
3. Extract data: (opcode & 0x0E00) ? (opcode >> 9) & 0x07 : 8
4. Read destination via CommandMode (cmd=0)
5. Perform add/subtract with size masking
6. Write result via CommandMode (cmd=1)
7. Set all 5 flags with size-specific casts
8. Also set EXTEND flag via setxtend()
```

---

## Carry/Overflow Algorithms

### gen_carry() - Arithmetic Carry Detection

Sign-based carry flag computation:

```
Returns 1 (carry occurred) if:
  (source < 0 AND destination >= 0)     // Negative+Positive=overflow
  OR (result < 0 AND destination >= 0)  // Result crossed 0
  OR (source < 0 AND result < 0)        // Double negative
```

**WASM Pseudocode:**
```javascript
function gen_carry(s, d, r) {
  return ((s < 0) && (!(d < 0))) || 
         ((r < 0) && (!(d < 0))) || 
         ((s < 0) && (r < 0)) ? 1 : 0;
}
```

### gen_over() - Signed Overflow Detection

Two's-complement overflow when operand signs contradict result sign:

```
Returns 1 (overflow occurred) if:
  (source >= 0 AND destination < 0 AND result >= 0)    // ++-
  OR (source < 0 AND destination >= 0 AND result < 0)  // -+-
```

**WASM Pseudocode:**
```javascript
function gen_over(s, d, r) {
  return ((!(s < 0)) && (d < 0) && (!(r < 0))) || 
         ((s < 0) && (!(d < 0)) && (r < 0)) ? 1 : 0;
}
```

---

## Flag Setting Order (Critical)

All instructions follow this order:

1. **Logical ops (ORI, ANDI, EORI):**
   - Conditional: NEG, ZERO
   - Always: OVER0, CARRY0

2. **Arithmetic ops (ADDI, SUBI, ADDQ, SUBQ):**
   - setcarry(gen_carry(...))
   - setover(gen_over(...))
   - Conditional: NEG, ZERO
   - ADDQ/SUBQ also: setxtend(gen_carry(...))

3. **Compare ops (CMPI, CMPA):**
   - setcarry(gen_carry(...))
   - setover(gen_over(...))
   - Conditional: NEG, ZERO

---

## CommandMode Dispatch Details

### When CommandMode is Called

**READ phase (cmd=0):**
```c
spc = cpu.pc;
work.destination = CommandMode[of.general.modesrc](
    of.general.regsrc, 0, 0L, of.special.size
);
cpu.pc = spc;  // MUST restore - CommandMode may advance PC!
```

**WRITE phase (cmd=1):**
```c
CommandMode[of.general.modesrc](
    of.general.regsrc, 1, work.result, of.special.size
);
```

### Size Parameter Meanings

- `size=0`: Byte (8-bit) - masks to 0x000000FF
- `size=1`: Word (16-bit) - masks to 0x0000FFFF
- `size=2`: Long (32-bit) - no mask

---

## PC Advancement Rules

**Opcode:** Always +2

**Immediate values:**
- Byte/Word immediate: +2 additional
- Long immediate: +4 additional

**Address register operations (CMPA):** +2 only (no immediate)

**Quick operations (ADDQ, SUBQ):** +2 only (immediate in opcode)

---

## WASM Conversion Checklist

### Phase 1: Basic Structure
- [ ] Port opcode union (use shift/mask instead of bitfields)
- [ ] Port work structure
- [ ] Map CPU state (pc, sregs.ccr, aregs.a[8])
- [ ] Implement flag macros as helper functions

### Phase 2: Helper Functions
- [ ] Implement gen_carry()
- [ ] Implement gen_over()
- [ ] Implement flag setters (setcarry, setover, setxtend)

### Phase 3: Memory Access
- [ ] Implement GETword() with endian swap
- [ ] Implement GETdword() with endian swap
- [ ] Implement PUTword() with endian swap
- [ ] Implement PUTdword() with endian swap

### Phase 4: Addressing Modes
- [ ] Create CommandMode stub functions
- [ ] Implement addressing mode decoders (DRD, ARD, ARI, etc.)
- [ ] Test read/write dispatch

### Phase 5: Instruction Implementation
- [ ] Port COM_ori/COM_andi/COM_eori (logical)
- [ ] Port COM_addi/COM_subi (arithmetic)
- [ ] Port COM_cmpi/COM_cmpa (compare)
- [ ] Port COM_addq/COM_subq (quick)

### Phase 6: Testing
- [ ] Unit tests for gen_carry/gen_over
- [ ] Unit tests for each instruction variant (byte/word/long)
- [ ] Integration tests with sample opcodes
- [ ] Verify flag behavior matches 68020 spec

---

## File References

| File | Purpose | Critical | Status |
|------|---------|----------|--------|
| EVMSim/Stcom.c | Main instruction handlers | Yes | Extracted |
| EVMSim/STFLAGS.C | Flag calculation functions | Yes | Extracted |
| EVMSim/macros.h | Flag manipulation macros | Yes | Extracted |
| EVMSim/STMEM.H | Memory access declarations | Yes | Extracted |
| EVMSim/STFLAGS.H | Flag function declarations | Yes | Extracted |
| EVMSim/STEACALC.H | Addressing mode interface | No | Referenced |
| EVMSim/STMAIN.H | CPU state definitions | No | Referenced |

---

## Known Implementation Details

### Opcode Union Fields

**General format (most instructions):**
- `regdest` [14:12] - Destination register (3 bits)
- `modedest` [11:9] - Destination mode (3 bits) [Used by CMPA only]
- `modesrc` [8:6] - Source addressing mode (3 bits)
- `regsrc` [2:0] - Source register (3 bits)
- `group` [15:12] - Opcode group (4 bits)

**Special format (immediate instructions):**
- `size` [7:6] - Operand size: 0=byte, 1=word, 2=long
- `modesrc` [8:6] - Destination addressing mode
- `regsrc` [2:0] - Destination register

### Quick Immediate Encoding (ADDQ/SUBQ)

- Bits [11:9] contain data (1-7)
- If bits [11:9] = 0, then data = 8
- Can be extracted as: `data = (opcode & 0x0E00) ? ((opcode >> 9) & 0x07) : 8`

### Address Register Sizing (CMPA)

- `modedest=3`: Word (16-bit signed)
- `modedest=7`: Long (32-bit signed)
- No byte operations for CMPA

---

## Related WASM Project Files

- `evm-web/evm-core/src/cpu.h` - CPU state definitions
- `evm-web/evm-core/src/cpu.c` - Core simulation loop
- `evm-web/evm-core/src/opcodes.c` - (Will contain ported instructions)

---

## Next Steps

1. **Immediate:** Use MC68020_INSTRUCTION_EXTRACT.md to begin WASM conversion
2. **Short term:** Complete Phase 1-3 of WASM checklist
3. **Medium term:** Port all 9 instructions with unit tests
4. **Long term:** Expand to full MC68020 instruction set

---

**Document created:** November 4, 2025  
**Status:** Complete and ready for WASM integration  
**Contact:** See EVM/CLAUDE.md for project guidelines
