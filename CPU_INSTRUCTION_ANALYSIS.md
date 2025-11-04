# MC68020 CPU Instruction Implementation Analysis

**Repository**: `/home/kgerlich/dev/EVM/evm-web/evm-core`

## Summary

The EVM simulator uses a **65,536-entry opcode jump table** (`Operation[]`) to dispatch CPU instructions. Each opcode (0x0000-0xFFFF) maps directly to a handler function. Instruction handlers are implemented in several source files and follow a consistent pattern for decoding operands, executing logic, and updating CPU state and flags.

---

## 1. CPU INSTRUCTION HANDLERS IMPLEMENTED

### Fully Implemented Instructions (24+)

Located in `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c`:

| Instruction | Line # | Opcode Range | Notes |
|---|---|---|---|
| **COM_add** | 40 | 0xD000-0xDFFF | Add with size variants (byte/word/long) |
| **COM_and** | 129 | 0xC000-0xCFFF | AND logic operation |
| **COM_beq** | 208 | 0x6700 | Branch if Equal |
| **COM_bne** | 229 | 0x6600 | Branch if Not Equal |
| **COM_bra** | 250 | 0x6000-0x60FF | Unconditional Branch |
| **COM_clr** | 268 | 0x4200 | Clear (write zero to destination) |
| **COM_cmp** | 282 | 0xB000-0xBFFF | Compare (affects flags, no result) |
| **COM_eor** | 326 | 0xB800-0xBFFF | Exclusive OR |
| **COM_jmp** | 371 | 0x4E80-0x4EFF (modes 2,5,7) | Jump (no return address saved) |
| **COM_jsr** | 406 | 0x4E90-0x4EFF | Jump to Subroutine (saves return address) |
| **COM_lea** | 608 | 0x41C0-0x41FF | Load Effective Address |
| **COM_movefromSR** | 821 | 0x40C0 | Move from Status Register |
| **COM_movequick** | 835 | 0x7000-0x7FFF | Move Quick (immediate to register) |
| **COM_movetoSR** | 843 | 0x46C0 | Move to Status Register |
| **COM_neg** | 858 | 0x4400 | Negate (two's complement) |
| **COM_nop** | 933 | 0x4E71 | No Operation |
| **COM_not** | 940 | 0x4600 | Bitwise NOT |
| **COM_or** | 985 | 0x8000-0x8FFF | OR logic operation |
| **COM_pea** | 1059 | 0x4840 | Push Effective Address |
| **COM_reset** | 1265 | 0x4E70 | Reset (peripherals) |
| **COM_rte** | 1280 | 0x4E73 | Return from Exception |
| **COM_rts** | 1304 | 0x4E75 | Return from Subroutine |
| **COM_sub** | 1312 | 0x9000-0x9FFF | Subtract |
| **COM_tst** | 1395 | 0x4A00 | Test (set flags, no result) |

### Stub Instructions (Minimal Implementation)

Located in `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c` (lines 1439-1450):

```c
void COM_mul(short opcode) { cpu.pc += 2; }
void COM_mulu(short opcode) { cpu.pc += 2; }
void COM_muls(short opcode) { cpu.pc += 2; }
void COM_mul020(short opcode) { cpu.pc += 2; }
void COM_div(short opcode) { cpu.pc += 2; }
void COM_divx(short opcode) { cpu.pc += 2; }
// ... 70+ more stubs
```

**Note**: Over 70 stub handlers exist in `instruction_stubs.c` (lines 39-127) that just increment PC by 2.

---

## 2. INSTRUCTION IMPLEMENTATION PATTERN

### Standard Handler Signature

```c
void COM_<instruction_name>(short opcode)
{
    CACHEFUNCTION(COM_<instruction_name>);  // Optimization hint (currently no-op)
    
    // 1. INCREMENT PROGRAM COUNTER
    cpu.pc += 2;  // Move to next opcode
    
    // 2. DECODE OPERANDS (using 'of' union)
    // The global union 'of' contains the opcode broken into fields:
    // - of.general.regsrc   (bits 2-0: source register)
    // - of.general.modesrc  (bits 5-3: source addressing mode)
    // - of.general.regdest  (bits 11-9: destination register)
    // - of.general.modedest (bits 8-6: destination mode)
    
    // 3. EXECUTE LOGIC
    switch(of.general.modedest)
    {
        case 0:  // Mode 0: Data Register Direct
            work.source = CommandMode[of.general.modesrc](...)
            // Perform operation
            break;
        case 1:  // Mode 1: Data Register Word
            // ... similar pattern with different size
            break;
        // ... more cases for other modes
    }
    
    // 4. UPDATE FLAGS
    if(result == 0) ZERO1;      // Set ZERO flag
    else ZERO0;                 // Clear ZERO flag
    if(result < 0) NEG1;        // Set NEGATIVE flag
    else NEG0;                  // Clear NEGATIVE flag
    // CARRY0, OVER0, OVER1, CARRY1, XTEND0, XTEND1 for other flags
}
```

### Key Components in Pattern

#### **A. Opcode Decoding Union** (defined in cpu_instructions.c:19-35)
```c
extern union {
    unsigned short o;              // Raw 16-bit opcode
    struct {
        unsigned regsrc:3;          // Bits 2-0: source register (0-7)
        unsigned modesrc:3;         // Bits 5-3: source addressing mode (0-7)
        unsigned modedest:3;        // Bits 8-6: destination mode
        unsigned regdest:3;         // Bits 11-9: destination register
        unsigned group:4;           // Bits 15-12: instruction group
    } general;
    struct {
        unsigned regsrc:3;
        unsigned modesrc:3;
        unsigned size:2;            // Bits 7-6: operation size (0=byte, 1=word, 2=long)
        unsigned reserved:4;
        unsigned group:4;
    } special;
} of;
```

#### **B. Memory Access Functions** (STMEM.H, lines 18-31)
```c
extern char GETbyte(unsigned long address);      // Read 8-bit
extern short GETword(unsigned long address);     // Read 16-bit
extern long GETdword(unsigned long address);     // Read 32-bit

extern void PUTbyte(unsigned long address, char data);      // Write 8-bit
extern void PUTword(unsigned long address, short data);     // Write 16-bit
extern void PUTdword(unsigned long address, long data);     // Write 32-bit
```

#### **C. Flag Manipulation Macros** (STFLAGS.H, lines 54-63)
```c
#define NEG0        setneg(0)       // Clear NEGATIVE flag
#define NEG1        setneg(1)       // Set NEGATIVE flag
#define ZERO0       setzero(0)      // Clear ZERO flag
#define ZERO1       setzero(1)      // Set ZERO flag
#define CARRY0      setcarry(0)     // Clear CARRY flag
#define CARRY1      setcarry(1)     // Set CARRY flag
#define OVER0       setover(0)      // Clear OVERFLOW flag
#define OVER1       setover(1)      // Set OVERFLOW flag
#define XTEND0      setxtend(0)     // Clear EXTEND flag
#define XTEND1      setxtend(1)     // Set EXTEND flag
```

#### **D. Addressing Mode Handler** (in CommandMode array)
Instructions use the `CommandMode[]` function pointer array to fetch/store operands based on addressing mode:
```c
extern long (*CommandMode[8])(char reg, char command, long destination, char size);
// command: 0 = READ, 1 = WRITE
// size: 0 = byte, 1 = word, 2 = dword
// Returns operand value or status
```

#### **E. Work Structure** (STSTDDEF.H, lines 50-54)
```c
struct tag_work {
    LONG source;        // Source operand
    LONG destination;   // Destination operand
    LONG result;        // Operation result (temp storage)
};
extern struct tag_work work;
```

---

## 3. OPCODE DISPATCH MECHANISM

### Main Simulation Loop

**File**: `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions_wrapper.c` (lines 5265-5310)

```c
void Simulate68k(unsigned long ops)
{
    while(ops--)
    {
        // CHECK FOR ADDRESS ERROR (odd PC)
        if(cpu.pc & 0x00000001L)
        {
            addr_err();
            continue;
        }
        
        // FETCH OPCODE
        of.o = GETword(cpu.pc);  // Load 16-bit opcode into union
        
        // SETUP STACK POINTER BASED ON CPU MODE
        switch(cpu.sregs.sr & 0x3000)  // Check privilege bits
        {
            case 0:
            case 0x1000:
                cpu.aregs.a[7] = cpu.usp;  // User mode
                break;
            case 0x2000:
                cpu.aregs.a[7] = cpu.ssp;  // Supervisor mode
                break;
            case 0x3000:
                cpu.aregs.a[7] = cpu.msp;  // Master mode
        }
        
        // SAVE PC FOR EXCEPTION HANDLING
        pcbefore = cpu.pc;
        
        // DISPATCH TO OPCODE HANDLER VIA JUMP TABLE
        if(!bStopped)
            Operation[of.o](of.o);  // Direct index into 65,536-entry table
        
        // CHECK FOR STOP INSTRUCTION
        if(of.o == 0x4E72 && pcbefore == cpu.pc)
            bStopped = TRUE;
        
        // ... update shadow stack pointers and check for interrupts
    }
}
```

### Opcode Table Definition

**File**: `/home/kgerlich/dev/EVM/evm-web/evm-core/src/sttable.c` (lines 95-65630)

```c
// Line 95: Begin 65,536-entry function pointer array
void (*Operation[])(short) =
{
    COM_ori,      // $0000
    COM_ori,      // $0001
    // ... (repeated with appropriate handler for each opcode)
    COM_jsr,      // $4E90
    COM_jsr,      // $4E91
    // ... up to $FFFF
};
```

### JSR/JMP Dispatch

**File**: sttable.c lines 20193-20239 show opcode mapping:

```c
COM_illegal,    // $4E80-$4E8F (reserved/illegal)
// ...
COM_jsr,        // $4E90-$4EAF (JSR variants with different addressing modes)
COM_jmp,        // $4EC0+ (JMP variants)
```

**Real files with full JSR/JMP implementation**:
- `cpu_instructions.c` (lines 371-504)
- `cpu_instructions_real.c` (lines 1769-1870)
- `cpu_instructions_wrapper.c` (lines 2069-2282)

---

## 4. UNIMPLEMENTED OPCODES

### JSR Status: **IMPLEMENTED** ✓

**Opcode Range**: 0x4E90-0x4EBF (addressing modes 2, 5, 6)
**File**: cpu_instructions.c, lines 406-504
**Status**: Full implementation with all addressing modes

JSR Addressing Modes Supported:
- Mode 2: Address Register Indirect `(An)`
- Mode 5: Address Register Indirect with Displacement `(d16,An)`
- Mode 6: Address Register Indirect with Index `(d8,An,Xn)` (both 68000 and 68020 formats)

### JMP Status: **IMPLEMENTED** ✓

**Opcode Range**: 0x4E80-0x4EFF (addressing modes 2, 5, 7)
**File**: cpu_instructions.c, lines 371-403
**Status**: Full implementation

### Partially/Not Implemented

| Instruction | Opcode | Status | Issue |
|---|---|---|---|
| MUL/MULU/MULS | 0xC0C0-0xCFFF | Stub only | Just increments PC |
| DIV/DIVX | 0x80C0-0x8FFF | Stub only | Just increments PC |
| ABCD | 0xC100-0xC10F | Stub | Not implemented |
| SBCD | 0x8100-0x810F | Stub | Not implemented |
| PACK | 0x8140-0x814F | Stub | Not implemented |
| UNPACK | 0x8180-0x818F | Stub | Not implemented |
| BITFIELD | 0xE8C0-0xEFFF | Stub | Not implemented |
| LINE A/F | 0xA000/0xF000 | Stub | Privilege violations |

Total Stubs: **70+ instructions** (most just do `cpu.pc += 2`)

---

## 5. TEMPLATE FOR NEW INSTRUCTION

### Simple Template (e.g., NOP pattern)

**File**: cpu_instructions.c, lines 933-937

```c
void COM_nop(short opcode)
{
    CACHEFUNCTION(COM_nop);
    cpu.pc += 2;
    // No operation - no flags affected
}
```

### Complex Template (e.g., ADD pattern)

**File**: cpu_instructions.c, lines 40-126

```c
void COM_add(short opcode)
{
    CACHEFUNCTION(COM_add);
    cpu.pc += 2;
    
    switch(of.general.modedest)
    {
        case 0:  // 8-bit operation
            work.source = CommandMode[of.general.modesrc](
                (char)(of.general.regsrc), 0, 0L, 0);
            cpu.dregs.d[of.general.regdest] = 
                (cpu.dregs.d[of.general.regdest]) +
                (unsigned char)(work.source & 0x000000ffL);
            if(cpu.dregs.d[of.general.regdest] == 0) ZERO1;
            else ZERO0;
            if(cpu.dregs.d[of.general.regdest] < 0) NEG1;
            else NEG0;
            break;
            
        case 1:  // 16-bit operation
            work.source = CommandMode[of.general.modesrc](
                (char)(of.general.regsrc), 0, 0L, 1);
            cpu.dregs.d[of.general.regdest] =
                (cpu.dregs.d[of.general.regdest] & 0xFFFF0000L) |
                (((cpu.dregs.d[of.general.regdest]) +
                (unsigned short)(work.source & 0x0000ffff)) & 0x0000FFFFL);
            if(cpu.dregs.d[of.general.regdest] == 0) ZERO1;
            else ZERO0;
            if(cpu.dregs.d[of.general.regdest] < 0) NEG1;
            else NEG0;
            break;
            
        case 2:  // 32-bit operation
            work.source = CommandMode[of.general.modesrc](
                (char)(of.general.regsrc), 0, 0L, 2);
            cpu.dregs.d[of.general.regdest] =
                cpu.dregs.d[of.general.regdest] + work.source;
            if(cpu.dregs.d[of.general.regdest] == 0) ZERO1;
            else ZERO0;
            if(cpu.dregs.d[of.general.regdest] < 0) NEG1;
            else NEG0;
            break;
            
        // Additional modes for address registers or memory destinations...
    }
}
```

### Branch Instruction Template

**File**: cpu_instructions.c, lines 250-265 (BRA)

```c
void COM_bra(short opcode)
{
    CACHEFUNCTION(COM_bra);
    
    // Decode displacement from opcode
    switch(opcode & 0x00ff)  // Lower byte holds displacement field
    {
        case 0:
            // Word displacement - read next word
            cpu.pc = cpu.pc + (short)GETword(cpu.pc+2) + 2;
            break;
        case 0xFF:
            // Long displacement - read next dword
            cpu.pc = cpu.pc + (long)GETdword(cpu.pc+2) + 2;
            break;
        default:
            // Byte displacement (sign-extended to 32 bits)
            cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            break;
    }
}
```

### JSR/Return Pattern

**File**: cpu_instructions.c, lines 406-504 (JSR)

```c
void COM_jsr(short opcode)
{
    short extension;
    long ea, od, bd, index;
    
    CACHEFUNCTION(COM_jsr);
    cpu.pc += 2;  // PC now points past opcode
    
    switch(of.general.modesrc)
    {
        case 2:  // ARI: (An)
            cpu.aregs.a[7] -= 4;              // Push on stack
            PUTdword(cpu.aregs.a[7], cpu.pc); // Save return address
            cpu.pc = cpu.aregs.a[(of.general.regsrc)];  // Jump
            break;
            
        case 5:  // ARID: (d16,An)
            cpu.aregs.a[7] -= 4;
            PUTdword(cpu.aregs.a[7], cpu.pc);
            cpu.pc += (long)GETword(cpu.pc);  // Add displacement
            break;
            
        case 6:  // ARII: (d8,An,Xn)
            cpu.aregs.a[7] -= 4;
            extension = GETword(cpu.pc);
            cpu.pc += 2;
            // ... complex index calculation
            break;
    }
}
```

---

## 6. INSTRUCTION EXAMPLES WITH LINE NUMBERS

### Example 1: Simple Instruction (NOP)

| Item | Details |
|---|---|
| **File** | `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c` |
| **Lines** | 933-937 |
| **Opcode** | 0x4E71 |
| **Code** | See template above |

### Example 2: Arithmetic Instruction (ADD)

| Item | Details |
|---|---|
| **File** | `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c` |
| **Lines** | 40-126 |
| **Opcode** | 0xD000-0xDFFF |
| **Pattern** | Switch on modedest (8 cases for different sizes/destinations) |

### Example 3: Jump Instruction (JSR)

| Item | Details |
|---|---|
| **File** | `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c` |
| **Lines** | 406-504 |
| **Opcode** | 0x4E90-0x4EBF |
| **Pattern** | Switch on modesrc (3 addressing modes) |

### Example 4: Branch Instruction (BRA)

| Item | Details |
|---|---|
| **File** | `/home/kgerlich/dev/EVM/evm-web/evm-core/src/cpu_instructions.c` |
| **Lines** | 250-265 |
| **Opcode** | 0x6000-0x60FF |
| **Pattern** | Displacement extraction from opcode vs extension words |

---

## 7. KEY IMPLEMENTATION DETAILS

### PC (Program Counter) Management

All instruction handlers follow this pattern:
1. `cpu.pc += 2` - Skip past opcode word
2. If opcode references memory operands, adjust PC again:
   - Extension word: `cpu.pc += 2`
   - Immediate data: `cpu.pc += 2` or `cpu.pc += 4` depending on size
   - Displacement: handled in GETword/GETdword calls

### Addressing Mode Decoding

The CPU uses 8 addressing modes encoded in the opcode:
- **0**: Dn (Data register direct)
- **1**: An (Address register direct)
- **2**: (An) (Address register indirect)
- **3**: (An)+ (Postincrement)
- **4**: -(An) (Predecrement)
- **5**: (d16,An) (Displacement)
- **6**: (d8,An,Xn) (Index, supports both 68000 and 68020 formats)
- **7**: Special (PC-relative, absolute, immediate)

### Size Encoding

Size is encoded differently depending on instruction type:
- **Byte operations**: Use modedest case 0
- **Word operations**: Use modedest case 1
- **Long operations**: Use modedest case 2
- Some instructions use `of.special.size` field (bits 7-6)

### Flag Setting

All arithmetic/logic instructions must update flags:
- **ZERO flag**: Set if result == 0
- **NEGATIVE flag**: Set if result < 0
- **CARRY flag**: Set if unsigned overflow
- **OVERFLOW flag**: Set if signed overflow
- **EXTEND flag**: Preserved from previous carry (for multi-word ops)

---

## 8. FILE ORGANIZATION

| Component | File Location | Lines | Purpose |
|---|---|---|---|
| **Instruction Implementations** | `src/cpu_instructions.c` | 1-1450 | 24 full + stubs |
| **Opcode Jump Table** | `src/sttable.c` | 95-65630 | 65,536 entries |
| **Main Loop** | `src/cpu_instructions_wrapper.c` | 5265-5310 | Instruction execution loop |
| **Opcode Union Definition** | `src/cpu_instructions.c` | 19-35 | Decoding bitfields |
| **Memory Access** | `include/STMEM.H` | 18-31 | GETbyte/word/dword, PUTbyte/word/dword |
| **Flag Macros** | `include/STFLAGS.H` | 54-63 | NEG0/1, ZERO0/1, CARRY0/1, etc. |
| **CPU State Structure** | `include/STSTDDEF.H` | 35-47 | CPU registers, SR, stack pointers |
| **Flag Functions** | `include/STFLAGS.H` | 13-50 | setcarry(), setzero(), gen_carry(), etc. |
| **Stub Handlers** | `src/instruction_stubs.c` | 31-127 | 70+ minimal stubs |

---

## 9. INSTRUCTION STATUS SUMMARY

| Category | Count | Examples |
|---|---|---|
| **Fully Implemented** | 24 | ADD, AND, JSR, JMP, BRA, CMP, LEA, NOP, RTS |
| **Stubs (PC only)** | 70+ | MUL, DIV, PACK, UNPACK, BITFIELD, ABCD, SBCD |
| **Completely Missing** | 50+ | Floating point (if any), coprocessor instructions |
| **Total Opcodes** | 65,536 | Mapped via Operation[] table |

---

## 10. HOW TO ADD A NEW INSTRUCTION

1. **Choose instruction handler name**: Follow pattern `COM_<mnemonic>`
2. **Implement handler function** in `cpu_instructions.c`:
   - Copy template matching complexity level
   - Use `of.general` or `of.special` to decode operands
   - Call `CommandMode[mode](reg, cmd, dest, size)` for addressing
   - Use `GETbyte/word/dword()` and `PUTbyte/word/dword()` for memory
   - Update appropriate flags (ZERO1/0, NEG1/0, CARRY1/0, OVER1/0)
   - Always do `cpu.pc += 2` at start, plus additional for extension words

3. **Register opcodes in sttable.c**:
   - Add extern declaration at top (line ~49)
   - Map opcode ranges to handler in Operation[] array (line ~95 onwards)
   - Add comment with opcode hex value

4. **Example entries in Operation[]**:
   ```c
   extern void COM_jsr(short);
   extern void COM_jmp(short);
   
   void (*Operation[])(short) = {
       // ...
       COM_jsr,  // $4E90
       COM_jmp,  // $4EC0
   };
   ```

5. **Test**:
   - Compile and link with WASM build
   - Create test program with new instruction
   - Execute and verify registers/memory state matches expected behavior

