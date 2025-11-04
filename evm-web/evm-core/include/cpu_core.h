/*
 * cpu_core.h
 *
 * MC68020 CPU instruction handlers and execution engine
 * Extracted from Stcom.c, platform-independent
 */

#ifndef __CPU_CORE_H__
#define __CPU_CORE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CPU Core Types
 * ============================================================================ */

/* Memory operation size */
#define SIZE_BYTE  1
#define SIZE_WORD  2
#define SIZE_DWORD 4

/* ============================================================================
 * CPU State Access (from current simulator context)
 * ============================================================================ */

/* These functions access the global simulator context set by simulator_init() */

/**
 * Read byte from memory
 */
uint8_t cpu_get_byte(uint32_t addr);

/**
 * Read word (16-bit) from memory
 */
uint16_t cpu_get_word(uint32_t addr);

/**
 * Read dword (32-bit) from memory
 */
uint32_t cpu_get_dword(uint32_t addr);

/**
 * Write byte to memory
 */
void cpu_put_byte(uint32_t addr, uint8_t val);

/**
 * Write word (16-bit) to memory
 */
void cpu_put_word(uint32_t addr, uint16_t val);

/**
 * Write dword (32-bit) to memory
 */
void cpu_put_dword(uint32_t addr, uint32_t val);

/* ============================================================================
 * Instruction Handlers (all COM_* functions from Stcom.c)
 * ============================================================================ */

#define DECLARE_INSTRUCTION(name) void name(uint16_t opcode)

/* Immediate instructions */
DECLARE_INSTRUCTION(COM_ori);
DECLARE_INSTRUCTION(COM_andi);
DECLARE_INSTRUCTION(COM_subi);
DECLARE_INSTRUCTION(COM_addi);
DECLARE_INSTRUCTION(COM_eori);
DECLARE_INSTRUCTION(COM_cmpi);

/* Bit operations */
DECLARE_INSTRUCTION(COM_movemtoEA);
DECLARE_INSTRUCTION(COM_ext);
DECLARE_INSTRUCTION(COM_movep);
DECLARE_INSTRUCTION(COM_dyntstbit);
DECLARE_INSTRUCTION(COM_stattstbit);

/* Address/Transfer */
DECLARE_INSTRUCTION(COM_lea);
DECLARE_INSTRUCTION(COM_chk);

/* Status register */
DECLARE_INSTRUCTION(COM_movefromSR);
DECLARE_INSTRUCTION(COM_movetoCCR);
DECLARE_INSTRUCTION(COM_movetoSR);

/* Arithmetic/Logical */
DECLARE_INSTRUCTION(COM_negx);
DECLARE_INSTRUCTION(COM_clr);
DECLARE_INSTRUCTION(COM_neg);
DECLARE_INSTRUCTION(COM_not);
DECLARE_INSTRUCTION(COM_nbcd);
DECLARE_INSTRUCTION(COM_swap);
DECLARE_INSTRUCTION(COM_pea);
DECLARE_INSTRUCTION(COM_tst);

/* Special */
DECLARE_INSTRUCTION(COM_illegal);
DECLARE_INSTRUCTION(COM_tas);
DECLARE_INSTRUCTION(COM_div020);
DECLARE_INSTRUCTION(COM_mul020);

/* Memory move */
DECLARE_INSTRUCTION(COM_movemtoreg);

/* Jumps/Branches */
DECLARE_INSTRUCTION(COM_jsr);
DECLARE_INSTRUCTION(COM_jmp);

/* Stack */
DECLARE_INSTRUCTION(COM_link);
DECLARE_INSTRUCTION(COM_unlink);

/* Exceptions */
DECLARE_INSTRUCTION(COM_trap);
DECLARE_INSTRUCTION(COM_trapv);
DECLARE_INSTRUCTION(COM_illegal);

/* Control flow */
DECLARE_INSTRUCTION(COM_rte);
DECLARE_INSTRUCTION(COM_rts);
DECLARE_INSTRUCTION(COM_rtr);
DECLARE_INSTRUCTION(COM_moveUSP);

/* Specials */
DECLARE_INSTRUCTION(COM_reset);
DECLARE_INSTRUCTION(COM_nop);
DECLARE_INSTRUCTION(COM_stop);
DECLARE_INSTRUCTION(COM_movec);

/* Branches */
DECLARE_INSTRUCTION(COM_bra);
DECLARE_INSTRUCTION(COM_bsr);
DECLARE_INSTRUCTION(COM_bhi);
DECLARE_INSTRUCTION(COM_bls);
DECLARE_INSTRUCTION(COM_bcc);
DECLARE_INSTRUCTION(COM_bcs);
DECLARE_INSTRUCTION(COM_bne);
DECLARE_INSTRUCTION(COM_beq);
DECLARE_INSTRUCTION(COM_bvc);
DECLARE_INSTRUCTION(COM_bvs);
DECLARE_INSTRUCTION(COM_bpl);
DECLARE_INSTRUCTION(COM_bmi);
DECLARE_INSTRUCTION(COM_bge);
DECLARE_INSTRUCTION(COM_blt);
DECLARE_INSTRUCTION(COM_bgt);
DECLARE_INSTRUCTION(COM_ble);

/* Arithmetic operations */
DECLARE_INSTRUCTION(COM_or);
DECLARE_INSTRUCTION(COM_and);
DECLARE_INSTRUCTION(COM_sub);
DECLARE_INSTRUCTION(COM_add);
DECLARE_INSTRUCTION(COM_eor);
DECLARE_INSTRUCTION(COM_cmp);
DECLARE_INSTRUCTION(COM_moveq);
DECLARE_INSTRUCTION(COM_move);
DECLARE_INSTRUCTION(COM_movea);

/* Bit shifts/rotates */
DECLARE_INSTRUCTION(COM_asr);
DECLARE_INSTRUCTION(COM_asl);
DECLARE_INSTRUCTION(COM_lsr);
DECLARE_INSTRUCTION(COM_lsl);
DECLARE_INSTRUCTION(COM_ror);
DECLARE_INSTRUCTION(COM_rol);
DECLARE_INSTRUCTION(COM_roxr);
DECLARE_INSTRUCTION(COM_roxl);

/* Line A/F */
DECLARE_INSTRUCTION(COM_linea);
DECLARE_INSTRUCTION(COM_linef);

/* Packed decimal */
DECLARE_INSTRUCTION(COM_pack);
DECLARE_INSTRUCTION(COM_unpack);

#undef DECLARE_INSTRUCTION

/* ============================================================================
 * CPU Core Initialization
 * ============================================================================ */

/**
 * Initialize CPU instruction handlers
 * Builds the opcode jump table
 */
void cpu_init_instructions(void);

/**
 * Execute a single opcode
 *
 * Returns: Status code (0 = success)
 */
uint16_t cpu_execute_opcode(uint16_t opcode);

#ifdef __cplusplus
}
#endif

#endif /* __CPU_CORE_H__ */
