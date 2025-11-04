/*
 * cpu_core.c
 *
 * MC68020 CPU instruction execution engine
 * Platform-independent instruction handlers extracted from Stcom.c
 */

#include <stdio.h>
#include <string.h>
#include "../include/simulator.h"
#include "../include/cpu_core.h"

/* This file will be compiled with specific sections from Stcom.c
   We use conditional compilation to extract only CPU logic */

/* For now, we provide stub implementations that show the architecture.
   The actual instruction logic will be linked from Stcom.c with
   platform-specific parts removed. */

/* Opcode execution table - maps opcode ranges to handlers */
typedef void (*opcode_handler_t)(uint16_t);

static opcode_handler_t opcode_table[65536];

/* ============================================================================
 * Memory Access Wrappers (using global simulator context)
 * ============================================================================ */

uint8_t cpu_get_byte(uint32_t addr)
{
    simulator_t *sim = simulator_get_current();
    if (sim == NULL) return 0;
    return (uint8_t)simulator_read_memory(sim, addr, 1);
}

uint16_t cpu_get_word(uint32_t addr)
{
    simulator_t *sim = simulator_get_current();
    if (sim == NULL) return 0;
    return (uint16_t)simulator_read_memory(sim, addr, 2);
}

uint32_t cpu_get_dword(uint32_t addr)
{
    simulator_t *sim = simulator_get_current();
    if (sim == NULL) return 0;
    return simulator_read_memory(sim, addr, 4);
}

void cpu_put_byte(uint32_t addr, uint8_t val)
{
    simulator_t *sim = simulator_get_current();
    if (sim != NULL) {
        simulator_write_memory(sim, addr, val, 1);
    }
}

void cpu_put_word(uint32_t addr, uint16_t val)
{
    simulator_t *sim = simulator_get_current();
    if (sim != NULL) {
        simulator_write_memory(sim, addr, val, 2);
    }
}

void cpu_put_dword(uint32_t addr, uint32_t val)
{
    simulator_t *sim = simulator_get_current();
    if (sim != NULL) {
        simulator_write_memory(sim, addr, val, 4);
    }
}

/* ============================================================================
 * CPU Instruction Handlers (stubs - will be linked from Stcom.c)
 * ============================================================================ */

/* Instruction handler stubs (actual implementations will be linked from Stcom.c) */
#define STUB_INSTRUCTION(name) void name(uint16_t opcode) { \
    fprintf(stderr, "Instruction " #name " not implemented\n"); \
}

STUB_INSTRUCTION(COM_ori)
STUB_INSTRUCTION(COM_andi)
STUB_INSTRUCTION(COM_subi)
STUB_INSTRUCTION(COM_addi)
STUB_INSTRUCTION(COM_eori)
STUB_INSTRUCTION(COM_cmpi)
STUB_INSTRUCTION(COM_movemtoEA)
STUB_INSTRUCTION(COM_ext)
STUB_INSTRUCTION(COM_movep)
STUB_INSTRUCTION(COM_dyntstbit)
STUB_INSTRUCTION(COM_stattstbit)
STUB_INSTRUCTION(COM_lea)
STUB_INSTRUCTION(COM_chk)
STUB_INSTRUCTION(COM_movefromSR)
STUB_INSTRUCTION(COM_movetoCCR)
STUB_INSTRUCTION(COM_movetoSR)
STUB_INSTRUCTION(COM_negx)
STUB_INSTRUCTION(COM_clr)
STUB_INSTRUCTION(COM_neg)
STUB_INSTRUCTION(COM_not)
STUB_INSTRUCTION(COM_nbcd)
STUB_INSTRUCTION(COM_swap)
STUB_INSTRUCTION(COM_pea)
STUB_INSTRUCTION(COM_tst)
STUB_INSTRUCTION(COM_illegal)
STUB_INSTRUCTION(COM_tas)
STUB_INSTRUCTION(COM_div020)
STUB_INSTRUCTION(COM_mul020)
STUB_INSTRUCTION(COM_movemtoreg)
STUB_INSTRUCTION(COM_jsr)
STUB_INSTRUCTION(COM_jmp)
STUB_INSTRUCTION(COM_link)
STUB_INSTRUCTION(COM_unlink)
STUB_INSTRUCTION(COM_trap)
STUB_INSTRUCTION(COM_trapv)
STUB_INSTRUCTION(COM_rte)
STUB_INSTRUCTION(COM_rts)
STUB_INSTRUCTION(COM_rtr)
STUB_INSTRUCTION(COM_moveUSP)
STUB_INSTRUCTION(COM_reset)
STUB_INSTRUCTION(COM_nop)
STUB_INSTRUCTION(COM_stop)
STUB_INSTRUCTION(COM_movec)
STUB_INSTRUCTION(COM_bra)
STUB_INSTRUCTION(COM_bsr)
STUB_INSTRUCTION(COM_bhi)
STUB_INSTRUCTION(COM_bls)
STUB_INSTRUCTION(COM_bcc)
STUB_INSTRUCTION(COM_bcs)
STUB_INSTRUCTION(COM_bne)
STUB_INSTRUCTION(COM_beq)
STUB_INSTRUCTION(COM_bvc)
STUB_INSTRUCTION(COM_bvs)
STUB_INSTRUCTION(COM_bpl)
STUB_INSTRUCTION(COM_bmi)
STUB_INSTRUCTION(COM_bge)
STUB_INSTRUCTION(COM_blt)
STUB_INSTRUCTION(COM_bgt)
STUB_INSTRUCTION(COM_ble)
STUB_INSTRUCTION(COM_or)
STUB_INSTRUCTION(COM_and)
STUB_INSTRUCTION(COM_sub)
STUB_INSTRUCTION(COM_add)
STUB_INSTRUCTION(COM_eor)
STUB_INSTRUCTION(COM_cmp)
STUB_INSTRUCTION(COM_moveq)
STUB_INSTRUCTION(COM_move)
STUB_INSTRUCTION(COM_movea)
STUB_INSTRUCTION(COM_asr)
STUB_INSTRUCTION(COM_asl)
STUB_INSTRUCTION(COM_lsr)
STUB_INSTRUCTION(COM_lsl)
STUB_INSTRUCTION(COM_ror)
STUB_INSTRUCTION(COM_rol)
STUB_INSTRUCTION(COM_roxr)
STUB_INSTRUCTION(COM_roxl)
STUB_INSTRUCTION(COM_linea)
STUB_INSTRUCTION(COM_linef)
STUB_INSTRUCTION(COM_pack)
STUB_INSTRUCTION(COM_unpack)

#undef STUB_INSTRUCTION

/* ============================================================================
 * Instruction Table Initialization
 * ============================================================================ */

/**
 * Build the 65536-entry opcode jump table
 * Maps each possible 16-bit opcode to its handler function
 * This replaces the Windows DLL-based sttable.c generation
 */
void cpu_init_instructions(void)
{
    /* For WASM, we build the table at runtime.
       For Windows, the existing sttable.c is used.
       We provide a minimal implementation here. */

    fprintf(stderr, "CPU instruction table initialization stub\n");
    fprintf(stderr, "Note: Actual implementation requires extracting from sttable.c\n");
}

/**
 * Execute a single opcode via the jump table
 */
uint16_t cpu_execute_opcode(uint16_t opcode)
{
    if (opcode_table[opcode]) {
        opcode_table[opcode](opcode);
        return 0;  /* Success */
    } else {
        fprintf(stderr, "Illegal opcode: 0x%04X\n", opcode);
        return 1;  /* Error */
    }
}
