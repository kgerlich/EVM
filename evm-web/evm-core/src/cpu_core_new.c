////////////////////////////////////////////////////////////////////////////////
// CPU Core Implementation
// Unified CPU core that integrates instruction handlers with simulator API
// Derived from Stcom.c, steacalc.c, Stexep.c, stmem.c
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "simulator.h"
#include "STFLAGS.H"
#include "STEACALC.H"
#include "STEXEP.H"
#include "macros.h"
#include "STSTDDEF.H"

// ============================================================================
// Global CPU state (from original Stcom.c)
// ============================================================================

CPU cpu;                                    // Virtual CPU register set
simulator_t *g_sim = NULL;                  // Current simulator context (shared with stmem.c)
BOOL bStopped = FALSE;                      // STOP instruction flag
struct tag_work work;                       // Work structure for calculations

long savepc, spc;                           // Temporary PC
long pcbefore;                              // PC before instruction
char AddressError = 0;                      // Address error flag

// Opcode decoding union
union {
    unsigned short o;                       // Opcode
    struct {
        unsigned regsrc   : 3;              // 0x0007
        unsigned modesrc  : 3;              // 0x0038
        unsigned modedest : 3;              // 0x01C0
        unsigned regdest  : 3;              // 0x0E00
        unsigned group    : 4;              // 0xF000
    }general;
    struct {
        unsigned regsrc   : 3;              // 0x0007
        unsigned modesrc  : 3;              // 0x0038
        unsigned size     : 2;              // 0x00C0
        unsigned reserved : 4;              // 0x0F00
        unsigned group    : 4;              // 0xF000
    }special;
}of;                                        // Opcode union

// EA calculation function pointer array
long (*CommandMode[8])(char reg, char command, long destination, char size) = {
    DRD, ARD, ARI, ARIPI, ARIPD, ARID, ARII, MISC
};

// ============================================================================
// Memory Access Functions (forward declarations - defined in stmem.c)
// ============================================================================

extern long GETbyte(long addr);
extern long GETword(long addr);
extern long GETdword(long addr);
extern void PUTbyte(long addr, long data);
extern void PUTword(long addr, long data);
extern void PUTdword(long addr, long data);

// ============================================================================
// Opcode Execution Dispatch
// ============================================================================

// Forward declare the operation jump table (from sttable.c)
// This jump table contains 65536 entries mapping opcodes to handler functions
extern void (*Operation[])(short opcode);

// ============================================================================
// Main Execution Loop
// ============================================================================

void cpu_execute_opcode(simulator_t *sim)
{
    if (sim == NULL) return;

    g_sim = sim;

    // CRITICAL: Sync simulator's CPU state to global cpu variable
    // The instruction handlers use the global 'cpu' variable, but we maintain state in sim->cpu
    cpu.pc = sim->cpu.pc;
    cpu.sregs.sr = sim->cpu.sr;
    cpu.usp = sim->cpu.usp;
    cpu.ssp = sim->cpu.ssp;
    cpu.msp = sim->cpu.msp;
    for (int i = 0; i < 8; i++) {
        cpu.dregs.d[i] = sim->cpu.d[i];
        cpu.aregs.a[i] = sim->cpu.a[i];
    }

    // Fetch opcode
    if (cpu.pc & 0x00000001L) {
        addr_err();  // Address error on odd PC
        return;
    }

    of.o = GETword(cpu.pc);

    // Setup stack pointer based on privilege mode
    switch (cpu.sregs.sr & 0x3000) {
        case 0:
        case 0x1000:
            cpu.aregs.a[7] = cpu.usp;
            break;
        case 0x2000:
            cpu.aregs.a[7] = cpu.ssp;
            break;
        case 0x3000:
            cpu.aregs.a[7] = cpu.msp;
    }

    // Save PC for exception handling
    pcbefore = cpu.pc;

    // Decode and execute opcode
    if (!bStopped) {
        Operation[of.o](of.o);
    }

    // Check for STOP instruction
    if (of.o == 0x4E72 && pcbefore == cpu.pc) {
        bStopped = TRUE;
    }

    // Update shadow stack pointers
    switch (cpu.sregs.sr & 0x3000) {
        case 0:
        case 0x1000:
            cpu.usp = cpu.aregs.a[7];
            break;
        case 0x2000:
            cpu.ssp = cpu.aregs.a[7];
            break;
        case 0x3000:
            cpu.msp = cpu.aregs.a[7];
    }

    // CRITICAL: Sync global cpu variable back to simulator's CPU state
    // This ensures that cpu_get_state() returns the updated state after instruction execution
    sim->cpu.pc = cpu.pc;
    sim->cpu.sr = cpu.sregs.sr;
    sim->cpu.usp = cpu.usp;
    sim->cpu.ssp = cpu.ssp;
    sim->cpu.msp = cpu.msp;
    for (int i = 0; i < 8; i++) {
        sim->cpu.d[i] = cpu.dregs.d[i];
        sim->cpu.a[i] = cpu.aregs.a[i];
    }

    // Call module simulation procedures
    for (int i = 0; i < sim->num_modules; i++) {
        if (sim->modules[i]->simulate) {
            sim->modules[i]->simulate(sim->modules[i]);
        }
    }
}

void cpu_execute_many(simulator_t *sim, unsigned long ops)
{
    if (sim == NULL) return;

    g_sim = sim;

    while (ops--) {
        cpu_execute_opcode(sim);
    }
}

// ============================================================================
// Initialization
// ============================================================================

void cpu_init_state(void)
{
    memset(&cpu, 0, sizeof(CPU));
    memset(&work, 0, sizeof(struct tag_work));

    cpu.pc = 0x000000;
    cpu.sregs.sr = 0x2700;      // Supervisor mode, IPL=7
    cpu.usp = 0x400000;         // User stack in RAM
    cpu.ssp = 0x410000;         // Supervisor stack in RAM
    cpu.msp = 0x420000;         // Master stack in RAM

    bStopped = FALSE;
    AddressError = 0;
}

simulator_t *cpu_get_current_simulator(void)
{
    return g_sim;
}

void cpu_set_current_simulator(simulator_t *sim)
{
    g_sim = sim;
}

