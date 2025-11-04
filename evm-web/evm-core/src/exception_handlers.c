/*
 * MC68020 Exception Handlers - WASM-compatible implementation
 * Based on original Stexep.c adapted for WebAssembly environment
 * Handles CPU exceptions and interrupts
 */

#include "STSTDDEF.H"
#include "STMEM.H"
#include "../include/simulator.h"

/* External references */
extern CPU cpu;
extern long spc;

/* Global state for exception handling */
static long pcbefore = 0;
static long savepc = 0;

/* Interrupt connection structure (simplified for WASM) */
struct {
    int ipl;                    /* Interrupt Priority Level */
    int VecNum;                 /* Vector Number */
    int bNonAutoVector;         /* Non-autovector flag */
} sConn_to_cpu = {0, 0x0f, 0};

/* Global counters */
static int bStopped = 0;
static int nIRQs = 0;

/*
 * Set the value to remember the PC from before instruction execution
 * Used for fault frame generation
 */
void set_pcbefore(long pc)
{
    pcbefore = pc;
    savepc = pc;
}

/*
 * NAME: void bus_err(void)
 * DESCRIPTION: Process bus error exception
 * Caused by missing memory/peripheral device
 */
void bus_err(void)
{
    if(cpu.pc == pcbefore) {
        /* Create short bus cycle fault stack frame */
        cpu.ssp -= 24;
        cpu.ssp -= 2;
        cpu_write_word(cpu.ssp, 0xa008);  /* FORMAT $A + vector offset */
    }
    else {
        /* Create long bus cycle fault stack frame */
        cpu.ssp -= 24;
        cpu.ssp -= 2;
        cpu_write_word(cpu.ssp, 0xb008);  /* FORMAT $B + vector offset */
    }

    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);     /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for bus error exception (vector #3) */
    cpu.pc = cpu_read_dword((long)(0x08 & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void addr_err(void)
 * DESCRIPTION: Process address error exception
 * Caused by uneven memory access
 */
void addr_err(void)
{
    if(cpu.pc == pcbefore) {
        /* Create short bus cycle fault stack frame */
        cpu.ssp -= 24;
        cpu.ssp -= 2;
        cpu_write_word(cpu.ssp, 0xa00c);
    }
    else {
        /* Create long bus cycle fault stack frame */
        cpu.ssp -= 24;
        cpu.ssp -= 2;
        cpu_write_word(cpu.ssp, 0xb00c);
    }

    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);     /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for address error (vector #3) */
    cpu.pc = cpu_read_dword((long)(0x0c & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void priv_viol(void)
 * DESCRIPTION: Process privilege violation exception
 * Caused by restricted operations like ORI.W #imm,SR in user mode
 */
void priv_viol(void)
{
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, 0x0020);       /* FORMAT $0 + vector offset */
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);      /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for privilege violation (vector #8) */
    cpu.pc = cpu_read_dword((long)(0x20 & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void div_by_zero(void)
 * DESCRIPTION: Process divide by zero exception
 */
void div_by_zero(void)
{
    /* Set stack pointer according to privilege mode */
    if((cpu.sregs.sr & 0x2000) == 0) {
        cpu.usp = cpu.aregs.a[7];
    }
    else {
        cpu.ssp = cpu.aregs.a[7];
    }

    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, 0x2014);       /* FORMAT $2 + vector offset #14 */
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);      /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for divide by zero exception (vector #5) */
    cpu.pc = cpu_read_dword((long)(0x14 & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void single_step(void)
 * DESCRIPTION: Process trace exception (single step)
 * Caused by setting TRACE bits in SR
 */
void single_step(void)
{
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, pcbefore);
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, 0x2024);       /* FORMAT $A + vector offset #24 */
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);      /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for trace exception (vector #9) */
    cpu.pc = cpu_read_dword((long)(0x24 & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void illegal(void)
 * DESCRIPTION: Process illegal opcode exception
 * Caused by illegal opcodes
 */
void illegal(void)
{
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, pcbefore);
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, 0x2010);       /* FORMAT $A + vector offset */
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);      /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for illegal opcode exception (vector #4) */
    cpu.pc = cpu_read_dword((long)(0x10 & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void emulatelinea(void)
 * DESCRIPTION: Process LINE A exception
 * Caused by opcodes $A000-$AFFF
 */
void emulatelinea(void)
{
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, 0x0028);       /* FORMAT $0 + vector offset */
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);      /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for line A exception (vector #10) */
    cpu.pc = cpu_read_dword((long)(0x28 & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void emulatelinef(void)
 * DESCRIPTION: Process LINE F exception
 * Caused by opcodes $F000-$FFFF
 */
void emulatelinef(void)
{
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, 0x002c);       /* FORMAT $0 + vector offset */
    cpu.ssp -= 4;
    cpu_write_dword(cpu.ssp, cpu.pc);      /* Save current PC to stack */
    cpu.ssp -= 2;
    cpu_write_word(cpu.ssp, cpu.sregs.sr); /* Save SR to stack */

    /* Get PC for line F exception (vector #11) */
    cpu.pc = cpu_read_dword((long)(0x2c & 0x0FFF) + cpu.vbr);
    cpu.aregs.a[7] = cpu.ssp;              /* Setup stack for supervisor mode */
    cpu.sregs.sr = (cpu.sregs.sr & 0x07ff) | 0x2000; /* Setup SR for exception */
}

/*
 * NAME: void Unknown(short opcode)
 * DESCRIPTION: Process unknown/unimplemented opcode
 * Logs error and stops simulation
 */
void Unknown(short opcode)
{
    /* In WASM environment, we log the error via console or internal log
     * For now, just stop the simulation */
    bStopped = 1;
}

/*
 * NAME: void CheckForInt(void)
 * DESCRIPTION: Check if there's an interrupt to process
 */
void CheckForInt(void)
{
    /* Check if interrupt priority level > interrupt mask in SR
     * OR if non-maskable interrupt (NMI has ipl=7) */
    if(sConn_to_cpu.ipl > ((cpu.sregs.sr >> 8) & 0x0007) || sConn_to_cpu.ipl == 7) {
        bStopped = 0;
        nIRQs++;

        /* Handle non-autovector interrupt */
        if(sConn_to_cpu.bNonAutoVector) {
            if(sConn_to_cpu.VecNum != 0x0f) {
                /* Setup interrupt stack frame */
                cpu.ssp -= 2;
                cpu_write_word(cpu.ssp, (sConn_to_cpu.VecNum * 4));
                cpu.ssp -= 4;
                cpu_write_dword(cpu.ssp, cpu.pc);       /* Save PC */
                cpu.ssp -= 2;
                cpu_write_word(cpu.ssp, cpu.sregs.sr);  /* Save SR */

                /* Get PC from vector table */
                cpu.pc = cpu_read_dword((long)((sConn_to_cpu.VecNum * 4) + cpu.vbr));
                cpu.aregs.a[7] = cpu.ssp;
                cpu.sregs.sr = (cpu.sregs.sr & 0x00ff) | 0x2000 | (sConn_to_cpu.ipl << 8);
                sConn_to_cpu.ipl = 0;
                sConn_to_cpu.VecNum = 0x0f;
                sConn_to_cpu.bNonAutoVector = 0;
            }
            else {
                /* Default vector */
                cpu.ssp -= 2;
                cpu_write_word(cpu.ssp, 0x3c);
                cpu.ssp -= 4;
                cpu_write_dword(cpu.ssp, cpu.pc);       /* Save PC */
                cpu.ssp -= 2;
                cpu_write_word(cpu.ssp, cpu.sregs.sr);  /* Save SR */

                cpu.pc = cpu_read_dword((long)(0x3c + cpu.vbr));
                cpu.aregs.a[7] = cpu.ssp;
                cpu.sregs.sr = (cpu.sregs.sr & 0x00ff) | 0x2000 | (sConn_to_cpu.ipl << 8);
                sConn_to_cpu.ipl = 0;
                sConn_to_cpu.VecNum = 0x0f;
                sConn_to_cpu.bNonAutoVector = 0;
            }
        }
        /* Handle autovector interrupt */
        else {
            cpu.ssp -= 2;
            cpu_write_word(cpu.ssp, (sConn_to_cpu.ipl * 4) + 0x60);
            cpu.ssp -= 4;
            cpu_write_dword(cpu.ssp, cpu.pc);           /* Save PC */
            cpu.ssp -= 2;
            cpu_write_word(cpu.ssp, cpu.sregs.sr);      /* Save SR */

            /* Get PC from vector table */
            cpu.pc = cpu_read_dword((long)((sConn_to_cpu.ipl * 4 + 0x60) + cpu.vbr));
            cpu.aregs.a[7] = cpu.ssp;
            cpu.sregs.sr = (cpu.sregs.sr & 0x00ff) | 0x2000 | (sConn_to_cpu.ipl << 8);
            sConn_to_cpu.ipl = 0;
            sConn_to_cpu.bNonAutoVector = 0;
        }
    }
}
