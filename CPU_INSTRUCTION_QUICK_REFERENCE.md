# CPU Instruction Implementation - Quick Reference

## Key Files

| File | Purpose | Key Content |
|------|---------|-------------|
| `evm-web/evm-core/src/cpu_instructions.c` | Main handlers | 24 full implementations, opcode union |
| `evm-web/evm-core/src/sttable.c` | Opcode table | 65,536-entry jump table |
| `evm-web/evm-core/src/cpu_instructions_wrapper.c` | Main loop | Simulate68k() function (line 5265) |
| `evm-web/evm-core/include/STSTDDEF.H` | Type definitions | CPU struct, macros, CACHEFUNCTION |
| `evm-web/evm-core/include/STMEM.H` | Memory access | GETbyte/word/dword, PUTbyte/word/dword |
| `evm-web/evm-core/include/STFLAGS.H` | Flag operations | setcarry(), setzero(), gen_carry() |
| `evm-web/evm-core/src/instruction_stubs.c` | Stubs | 70+ minimal implementations |

## Opcode Dispatch Flow

```
Simulate68k() [wrapper.c:5265]
    └─> Fetch opcode with GETword()
    └─> Load into 'of' union
    └─> Operation[of.o](of.o)  [index into 65,536-entry table in sttable.c:95]
        └─> com_jsr(), COM_add(), COM_bra(), etc.
            └─> Decode of.general.modesrc, of.general.regdest, etc.
            └─> Perform operation
            └─> Update flags (NEG0/1, ZERO0/1, CARRY0/1, OVER0/1)
```

## Implemented Instructions (24+)

**Location**: `cpu_instructions.c`

```
ADD (line 40)      - 0xD000-0xDFFF
AND (line 129)     - 0xC000-0xCFFF
BEQ (line 208)     - 0x6700
BNE (line 229)     - 0x6600
BRA (line 250)     - 0x6000-0x60FF
CLR (line 268)     - 0x4200
CMP (line 282)     - 0xB000-0xBFFF
EOR (line 326)     - 0xB800-0xBFFF
JMP (line 371)     - 0x4E80-0x4EFF (modes 2,5,7)
JSR (line 406)     - 0x4E90-0x4EBF ✓ FULLY IMPLEMENTED
LEA (line 608)     - 0x41C0-0x41FF
MOVEFROMSR (821)   - 0x40C0
MOVEQUICK (835)    - 0x7000-0x7FFF
MOVETOSR (843)     - 0x46C0
NEG (line 858)     - 0x4400
NOP (line 933)     - 0x4E71
NOT (line 940)     - 0x4600
OR (line 985)      - 0x8000-0x8FFF
PEA (line 1059)    - 0x4840
RESET (line 1265)  - 0x4E70
RTE (line 1280)    - 0x4E73
RTS (line 1304)    - 0x4E75
SUB (line 1312)    - 0x9000-0x9FFF
TST (line 1395)    - 0x4A00
```

## Instruction Implementation Checklist

- [ ] Declare handler: `void COM_<name>(short opcode)`
- [ ] Add `CACHEFUNCTION(COM_<name>);` (no-op macro)
- [ ] Do `cpu.pc += 2;` to skip opcode word
- [ ] Decode using `of.general.*` or `of.special.*` bitfields
- [ ] Call `CommandMode[mode](reg, cmd, dest, size)` for addressing
- [ ] Use `GETbyte/word/dword()` and `PUTbyte/word/dword()` for memory
- [ ] Update flags with `NEG0/1`, `ZERO0/1`, `CARRY0/1`, `OVER0/1`
- [ ] Add `extern void COM_<name>(short);` in `sttable.c`
- [ ] Map opcodes in `Operation[]` array in `sttable.c`

## Code Templates

### Minimal (NOP-style)
```c
void COM_nop(short opcode)
{
    CACHEFUNCTION(COM_nop);
    cpu.pc += 2;
}
```

### Simple (with flags)
```c
void COM_tst(short opcode)
{
    CACHEFUNCTION(COM_tst);
    cpu.pc += 2;
    work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, of.special.size);
    if(work.source == 0) ZERO1; else ZERO0;
    if(work.source < 0) NEG1; else NEG0;
    CARRY0; OVER0;
}
```

### Complex (with addressing modes)
```c
void COM_add(short opcode)
{
    CACHEFUNCTION(COM_add);
    cpu.pc += 2;
    switch(of.general.modedest)
    {
        case 0:  // Byte
            work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 0);
            cpu.dregs.d[of.general.regdest] = (cpu.dregs.d[of.general.regdest]) + (unsigned char)(work.source & 0x000000ffL);
            if(cpu.dregs.d[of.general.regdest] == 0) ZERO1; else ZERO0;
            if(cpu.dregs.d[of.general.regdest] < 0) NEG1; else NEG0;
            break;
        case 1:  // Word
            // ... similar
            break;
        case 2:  // Long
            // ... similar
            break;
    }
}
```

### Branch (PC-relative)
```c
void COM_bra(short opcode)
{
    CACHEFUNCTION(COM_bra);
    switch(opcode & 0x00ff)
    {
        case 0:
            cpu.pc = cpu.pc + (short)GETword(cpu.pc+2) + 2;
            break;
        case 0xFF:
            cpu.pc = cpu.pc + (long)GETdword(cpu.pc+2) + 2;
            break;
        default:
            cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            break;
    }
}
```

### Subroutine (stack save)
```c
void COM_jsr(short opcode)
{
    CACHEFUNCTION(COM_jsr);
    cpu.pc += 2;
    switch(of.general.modesrc)
    {
        case 2:  // (An)
            cpu.aregs.a[7] -= 4;
            PUTdword(cpu.aregs.a[7], cpu.pc);
            cpu.pc = cpu.aregs.a[(of.general.regsrc)];
            break;
    }
}
```

## Key Data Structures

### CPU State
```c
typedef struct {
    struct { LONG a[8]; } aregs;    // A0-A7 (A7 = stack pointer)
    struct { LONG d[8]; } dregs;    // D0-D7
    LONG pc;                         // Program counter
    struct { USHORT sr; } sregs;    // Status register
    LONG usp, ssp, msp;             // User/Supervisor/Master stack pointers
} CPU;
extern CPU cpu;
```

### Opcode Union
```c
union {
    unsigned short o;               // Raw 16-bit opcode
    struct {
        unsigned regsrc:3;          // Bits 2-0
        unsigned modesrc:3;         // Bits 5-3
        unsigned modedest:3;        // Bits 8-6
        unsigned regdest:3;         // Bits 11-9
        unsigned group:4;           // Bits 15-12
    } general;
} of;
extern union { ... } of;
```

### Work Structure
```c
struct tag_work {
    LONG source;
    LONG destination;
    LONG result;
};
extern struct tag_work work;
```

## Flag Macros

| Macro | Purpose |
|-------|---------|
| `NEG0` / `NEG1` | Clear/set NEGATIVE flag (bit 3) |
| `ZERO0` / `ZERO1` | Clear/set ZERO flag (bit 2) |
| `CARRY0` / `CARRY1` | Clear/set CARRY flag (bit 0) |
| `OVER0` / `OVER1` | Clear/set OVERFLOW flag (bit 1) |
| `XTEND0` / `XTEND1` | Clear/set EXTEND flag (bit 4) |

## Addressing Modes

| Mode | Name | Example | Format |
|------|------|---------|--------|
| 0 | Dn (Data Register) | `ADD D1,D2` | Direct |
| 1 | An (Address Register) | `ADD A1,D2` | Direct |
| 2 | (An) | `ADD (A1),D2` | Indirect |
| 3 | (An)+ | `ADD (A1)+,D2` | Post-increment |
| 4 | -(An) | `ADD -(A1),D2` | Pre-decrement |
| 5 | (d16,An) | `ADD $10(A1),D2` | With displacement |
| 6 | (d8,An,Xn) | `ADD $10(A1,D1),D2` | With index |
| 7 | Special | PC-relative, absolute, immediate | Various |

## Memory Access Functions

```c
char GETbyte(unsigned long address);       // Read 8-bit
short GETword(unsigned long address);      // Read 16-bit
long GETdword(unsigned long address);      // Read 32-bit

void PUTbyte(unsigned long address, char data);      // Write 8-bit
void PUTword(unsigned long address, short data);     // Write 16-bit
void PUTdword(unsigned long address, long data);     // Write 32-bit
```

## Size Constants

```c
#define SIZE_BYTE   0   // 8-bit operation
#define SIZE_WORD   1   // 16-bit operation
#define SIZE_DWORD  2   // 32-bit operation
```

## CommandMode Array

```c
extern long (*CommandMode[8])(char reg, char command, long destination, char size);
// command: 0 = READ, 1 = WRITE
// size: 0 = byte, 1 = word, 2 = dword
// reg: register number (0-7)
// destination: address or value to write
// Returns: operand value (for reads) or status (for writes)
```

## Testing New Instructions

1. **Add handler** in `cpu_instructions.c`
2. **Register** in `sttable.c` extern and Operation[]
3. **Compile**: `cd evm-web/evm-core && ./build.sh`
4. **Create test** with assembly code using instruction
5. **Load and run** test program in simulator
6. **Verify** register/memory state is correct

## Common Mistakes

| Mistake | Fix |
|---------|-----|
| Forget `cpu.pc += 2` | All handlers must skip opcode |
| Wrong extension size | 68020 modes need extra PC adjustments |
| Don't update flags | All arithmetic must set NEG/ZERO/CARRY/OVER |
| Use wrong CommandMode | Mode parameter must match addressing mode |
| Forget extern in sttable.c | Won't compile if handler not declared |
| Wrong Operation[] index | Opcode value is direct array index |

## Example: Implementing ADDI (Add Immediate)

**Expected behavior**: ADD a 16/32-bit immediate value to a register

**Steps**:

1. Create handler in `cpu_instructions.c`:
```c
void COM_addi(short opcode)
{
    CACHEFUNCTION(COM_addi);
    cpu.pc += 2;
    
    // Immediate data follows opcode
    long immediate = GETword(cpu.pc);
    cpu.pc += 2;
    
    // Add to register
    cpu.dregs.d[0] += immediate;
    
    // Update flags
    if(cpu.dregs.d[0] == 0) ZERO1; else ZERO0;
    if(cpu.dregs.d[0] < 0) NEG1; else NEG0;
}
```

2. Register in `sttable.c` (after line 10):
```c
extern void COM_addi(short);
```

3. Map in Operation[] array (find 0x0600-0x06FF range):
```c
COM_addi,  // $0600
COM_addi,  // $0601
// ... etc for all variants
```

4. Rebuild and test!

