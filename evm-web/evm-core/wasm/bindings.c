#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations from main simulator */
extern void Simulate68k(unsigned long ops);
extern void SetupSim(void);
extern void ResetSim(void);
extern void ExitSim(void);
extern int CheckForInt(void);

/* CPU structure from Stcom.h */
typedef struct {
    unsigned long pc;           /* Program counter */
    unsigned short sr;          /* Status register */
    unsigned long dregs[8];     /* Data registers D0-D7 */
    unsigned long aregs[8];     /* Address registers A0-A7 */
    unsigned long ssp;          /* Supervisor stack pointer */
    unsigned long usp;          /* User stack pointer */
    unsigned long msp;          /* Master stack pointer */
} CPUState;

/* Global simulator state */
static int initialized = 0;
static int running = 0;

/* Memory access functions (from Stmem.c) */
extern unsigned char GETbyte(unsigned long addr);
extern void PUTbyte(unsigned long addr, unsigned char val);
extern unsigned short GETword(unsigned long addr);
extern void PUTword(unsigned long addr, unsigned short val);
extern unsigned long GETdword(unsigned long addr);
extern void PUTdword(unsigned long addr, unsigned long val);

/* CPU state access (from Stcom.c) */
extern struct {
    unsigned short sr;
    unsigned long d[8];
    unsigned long a[8];
    unsigned long pc;
    unsigned long ssp;
    unsigned long usp;
    unsigned long msp;
} cpu;

/**
 * Initialize the simulator
 */
EMSCRIPTEN_KEEPALIVE
int cpu_init() {
    if (initialized) {
        return 0;  /* Already initialized */
    }

    SetupSim();
    initialized = 1;
    running = 0;
    return 1;
}

/**
 * Reset the simulator to initial state
 */
EMSCRIPTEN_KEEPALIVE
void cpu_reset() {
    ResetSim();
    running = 0;
}

/**
 * Execute a single instruction
 */
EMSCRIPTEN_KEEPALIVE
void cpu_step() {
    if (!initialized) {
        cpu_init();
    }
    Simulate68k(1);
}

/**
 * Execute N instructions
 */
EMSCRIPTEN_KEEPALIVE
void cpu_run(unsigned long count) {
    if (!initialized) {
        cpu_init();
    }
    running = 1;
    Simulate68k(count);
}

/**
 * Pause execution
 */
EMSCRIPTEN_KEEPALIVE
void cpu_pause() {
    running = 0;
}

/**
 * Check if simulator is running
 */
EMSCRIPTEN_KEEPALIVE
int cpu_is_running() {
    return running;
}

/**
 * Get complete CPU state as JSON-compatible struct
 * Returns pointer to static CPUState structure
 */
EMSCRIPTEN_KEEPALIVE
CPUState* cpu_get_state() {
    static CPUState state;

    state.pc = cpu.pc;
    state.sr = cpu.sr;

    for (int i = 0; i < 8; i++) {
        state.dregs[i] = cpu.d[i];
        state.aregs[i] = cpu.a[i];
    }

    state.ssp = cpu.ssp;
    state.usp = cpu.usp;
    state.msp = cpu.msp;

    return &state;
}

/**
 * Set a data register value
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_dreg(int reg, unsigned long value) {
    if (reg >= 0 && reg < 8) {
        cpu.d[reg] = value;
    }
}

/**
 * Set an address register value
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_areg(int reg, unsigned long value) {
    if (reg >= 0 && reg < 8) {
        cpu.a[reg] = value;
    }
}

/**
 * Set program counter
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_pc(unsigned long pc) {
    cpu.pc = pc;
}

/**
 * Set status register
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_sr(unsigned short sr) {
    cpu.sr = sr;
}

/**
 * Read a byte from memory
 */
EMSCRIPTEN_KEEPALIVE
unsigned char cpu_read_byte(unsigned long addr) {
    return GETbyte(addr);
}

/**
 * Read a word (16-bit) from memory
 */
EMSCRIPTEN_KEEPALIVE
unsigned short cpu_read_word(unsigned long addr) {
    return GETword(addr);
}

/**
 * Read a dword (32-bit) from memory
 */
EMSCRIPTEN_KEEPALIVE
unsigned long cpu_read_dword(unsigned long addr) {
    return GETdword(addr);
}

/**
 * Read multiple bytes from memory (for memory inspector)
 * buf: pointer to output buffer
 * addr: starting address
 * size: number of bytes to read
 */
EMSCRIPTEN_KEEPALIVE
void cpu_read_memory_range(unsigned char* buf, unsigned long addr, unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
        buf[i] = GETbyte(addr + i);
    }
}

/**
 * Write a byte to memory
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_byte(unsigned long addr, unsigned char val) {
    PUTbyte(addr, val);
}

/**
 * Write a word (16-bit) to memory
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_word(unsigned long addr, unsigned short val) {
    PUTword(addr, val);
}

/**
 * Write a dword (32-bit) to memory
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_dword(unsigned long addr, unsigned long val) {
    PUTdword(addr, val);
}

/**
 * Write multiple bytes to memory
 * buf: pointer to input buffer
 * addr: starting address
 * size: number of bytes to write
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_memory_range(unsigned char* buf, unsigned long addr, unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
        PUTbyte(addr + i, buf[i]);
    }
}

/**
 * Load ROM from binary data
 * data: pointer to binary data
 * size: size in bytes
 */
EMSCRIPTEN_KEEPALIVE
void cpu_load_rom(unsigned char* data, unsigned long size) {
    /* ROM is mapped at 0x000000 */
    cpu_write_memory_range(data, 0x000000, size);
}

/**
 * Load program into RAM
 * data: pointer to binary data
 * size: size in bytes
 * addr: starting address (default 0x400000 for RAM)
 */
EMSCRIPTEN_KEEPALIVE
void cpu_load_program(unsigned char* data, unsigned long size, unsigned long addr) {
    cpu_write_memory_range(data, addr, size);
}

/**
 * Get memory size information
 */
EMSCRIPTEN_KEEPALIVE
unsigned long cpu_get_memory_size() {
    return 0x1000000;  /* 16MB address space */
}

/**
 * Cleanup simulator
 */
EMSCRIPTEN_KEEPALIVE
void cpu_cleanup() {
    if (initialized) {
        ExitSim();
        initialized = 0;
    }
}
