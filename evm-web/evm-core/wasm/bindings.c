/*
 * wasm/bindings.c
 *
 * Emscripten WASM bindings for the EVM simulator
 * Exposes the simulator API to JavaScript
 */

#include <emscripten.h>
#include <stdlib.h>
#include "../include/simulator.h"

/* Global simulator context */
static simulator_t *g_simulator = NULL;

/* ============================================================================
 * Simulator Lifecycle
 * ============================================================================ */

/**
 * Initialize the simulator (WITHOUT calling reset)
 *
 * Called from JavaScript before any other simulator operations.
 * NOTE: Does NOT call reset - reset should be called after ROM is loaded
 */
EMSCRIPTEN_KEEPALIVE
int cpu_init(void)
{
    if (g_simulator != NULL) {
        simulator_destroy(g_simulator);
    }

    g_simulator = simulator_init();
    if (g_simulator == NULL) {
        return 0;  /* Failure */
    }

    if (simulator_load_modules(g_simulator) != 0) {
        simulator_destroy(g_simulator);
        g_simulator = NULL;
        return 0;  /* Failure */
    }

    /* NOTE: Do NOT call simulator_reset here!
     * Reset must be called AFTER ROM is loaded, so reset vectors can be read from ROM.
     * The JavaScript side will call cpu_reset() after loading ROM.
     */

    return 1;  /* Success */
}

/**
 * Reset the simulator to initial state
 */
EMSCRIPTEN_KEEPALIVE
void cpu_reset(void)
{
    if (g_simulator != NULL) {
        simulator_reset(g_simulator);
    }
}

/**
 * Cleanup and destroy the simulator
 */
EMSCRIPTEN_KEEPALIVE
void cpu_shutdown(void)
{
    if (g_simulator != NULL) {
        simulator_destroy(g_simulator);
        g_simulator = NULL;
    }
}

/* ============================================================================
 * CPU Execution
 * ============================================================================ */

/**
 * Execute a single CPU instruction
 *
 * Returns: 0 on success, non-zero on error
 */
EMSCRIPTEN_KEEPALIVE
int cpu_step(void)
{
    if (g_simulator == NULL) return -1;
    return simulator_step(g_simulator);
}

/**
 * Execute N CPU instructions
 *
 * @param count Number of instructions to execute
 * @return Number of instructions actually executed
 */
EMSCRIPTEN_KEEPALIVE
uint32_t cpu_run(uint32_t count)
{
    if (g_simulator == NULL) return 0;
    return simulator_run(g_simulator, count);
}

/**
 * Pause simulator execution
 */
EMSCRIPTEN_KEEPALIVE
void cpu_pause(void)
{
    if (g_simulator != NULL) {
        simulator_pause(g_simulator);
    }
}

/* ============================================================================
 * CPU State Access
 * ============================================================================ */

/**
 * Get complete CPU state as packed structure
 *
 * Returns pointer to CPU state structure that can be read from WASM memory
 */
EMSCRIPTEN_KEEPALIVE
const simulator_cpu_state_t *cpu_get_state(void)
{
    if (g_simulator == NULL) return NULL;
    return simulator_get_state(g_simulator);
}

/**
 * Get program counter
 */
EMSCRIPTEN_KEEPALIVE
uint32_t cpu_get_pc(void)
{
    if (g_simulator == NULL) return 0;
    return simulator_get_state(g_simulator)->pc;
}

/**
 * Set program counter
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_pc(uint32_t value)
{
    if (g_simulator == NULL) return;
    g_simulator->cpu.pc = value & 0xFFFFFF;  /* 24-bit address space */
}

/**
 * Get data register value
 *
 * @param reg Register number (0-7)
 * @return Register value
 */
EMSCRIPTEN_KEEPALIVE
uint32_t cpu_get_dreg(int reg)
{
    if (g_simulator == NULL || reg < 0 || reg > 7) return 0;
    return g_simulator->cpu.d[reg];
}

/**
 * Set data register value
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_dreg(int reg, uint32_t value)
{
    if (g_simulator == NULL || reg < 0 || reg > 7) return;
    g_simulator->cpu.d[reg] = value;
}

/**
 * Get address register value
 *
 * @param reg Register number (0-7)
 * @return Register value
 */
EMSCRIPTEN_KEEPALIVE
uint32_t cpu_get_areg(int reg)
{
    if (g_simulator == NULL || reg < 0 || reg > 7) return 0;
    return g_simulator->cpu.a[reg];
}

/**
 * Set address register value
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_areg(int reg, uint32_t value)
{
    if (g_simulator == NULL || reg < 0 || reg > 7) return;
    simulator_set_register(g_simulator, reg, value, 'a');
}

/**
 * Get status register
 */
EMSCRIPTEN_KEEPALIVE
uint16_t cpu_get_sr(void)
{
    if (g_simulator == NULL) return 0;
    return g_simulator->cpu.sr;
}

/**
 * Set status register
 */
EMSCRIPTEN_KEEPALIVE
void cpu_set_sr(uint16_t value)
{
    if (g_simulator == NULL) return;
    g_simulator->cpu.sr = value;
}

/* ============================================================================
 * Memory Access
 * ============================================================================ */

/**
 * Read byte from memory
 *
 * @param addr Memory address
 * @return Byte value
 */
EMSCRIPTEN_KEEPALIVE
uint8_t cpu_read_byte(uint32_t addr)
{
    if (g_simulator == NULL) return 0;
    return (uint8_t)simulator_read_memory(g_simulator, addr, 1);
}

/**
 * Read word (16-bit) from memory
 */
EMSCRIPTEN_KEEPALIVE
uint16_t cpu_read_word(uint32_t addr)
{
    if (g_simulator == NULL) return 0;
    return (uint16_t)simulator_read_memory(g_simulator, addr, 2);
}

/**
 * Read dword (32-bit) from memory
 */
EMSCRIPTEN_KEEPALIVE
uint32_t cpu_read_dword(uint32_t addr)
{
    if (g_simulator == NULL) return 0;
    return simulator_read_memory(g_simulator, addr, 4);
}

/**
 * Write byte to memory
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_byte(uint32_t addr, uint8_t value)
{
    if (g_simulator != NULL) {
        simulator_write_memory(g_simulator, addr, value, 1);
    }
}

/**
 * Write word (16-bit) to memory
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_word(uint32_t addr, uint16_t value)
{
    if (g_simulator != NULL) {
        simulator_write_memory(g_simulator, addr, value, 2);
    }
}

/**
 * Write dword (32-bit) to memory
 */
EMSCRIPTEN_KEEPALIVE
void cpu_write_dword(uint32_t addr, uint32_t value)
{
    if (g_simulator != NULL) {
        simulator_write_memory(g_simulator, addr, value, 4);
    }
}

/* ============================================================================
 * Program Loading
 * ============================================================================ */

/**
 * Load program data into memory
 *
 * @param data Pointer to program data in WASM linear memory
 * @param size Size of program data in bytes
 * @param addr Base address to load at
 * @return 0 on success, -1 on error
 */
EMSCRIPTEN_KEEPALIVE
int cpu_load_program(uint8_t *data, uint32_t size, uint32_t addr)
{
    if (g_simulator == NULL || data == NULL) return -1;
    return simulator_load_program(g_simulator, data, size, addr);
}

/**
 * Load ROM image (S19 format or binary)
 *
 * @param data Pointer to ROM data
 * @param size Size of ROM data
 * @return 0 on success, -1 on error
 */
EMSCRIPTEN_KEEPALIVE
int cpu_load_rom(uint8_t *data, uint32_t size)
{
    if (g_simulator == NULL || data == NULL) return -1;
    /* Load ROM at address 0x000000 */
    return simulator_load_program(g_simulator, data, size, 0x000000);
}

/**
 * Direct ROM initialization - bypass permission checks for loading ROM at startup
 *
 * This function directly writes to ROM memory, bypassing the read-only check
 * that would normally prevent writes. This allows ROM to be loaded during
 * simulator initialization.
 *
 * @param data Pointer to ROM data in WASM linear memory
 * @param size Size of ROM data in bytes
 * @param addr Base address to load at (should be 0x000000)
 * @return 0 on success, -1 on error
 */
EMSCRIPTEN_KEEPALIVE
int cpu_init_rom(uint8_t *data, uint32_t size, uint32_t addr)
{
    if (g_simulator == NULL || data == NULL) return -1;

    /* Find the ROM module */
    for (int i = 0; i < g_simulator->num_modules; i++) {
        simulator_module_t *mod = g_simulator->modules[i];
        if (mod && mod->base_addr == 0x000000 && mod->size == 0x10000) {
            /* This is the ROM module - directly access its memory */
            typedef struct { uint8_t *memory; size_t size; int writable; } rom_state_t;
            rom_state_t *state = (rom_state_t *)mod->state;

            if (!state || !state->memory) return -1;

            uint32_t offset = addr - mod->base_addr;
            if (offset + size > state->size) {
                return -1;  /* Data doesn't fit in ROM */
            }

            /* Direct memory copy, bypassing write permission check */
            for (size_t i = 0; i < size; i++) {
                state->memory[offset + i] = data[i];
            }

            return 0;
        }
    }

    return -1;  /* ROM module not found */
}

/* ============================================================================
 * Debugging/Status
 * ============================================================================ */

/**
 * Get simulator status
 *
 * Returns: 1 if initialized and ready, 0 otherwise
 */
EMSCRIPTEN_KEEPALIVE
int cpu_is_initialized(void)
{
    return (g_simulator != NULL) ? 1 : 0;
}

/**
 * Get last error message (if any)
 */
EMSCRIPTEN_KEEPALIVE
const char *cpu_get_error(void)
{
    /* TODO: Implement error reporting system */
    return "No error";
}
