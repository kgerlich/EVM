/*
 * simulator.c
 *
 * Core EVM simulator implementation
 * Platform-independent execution engine for MC68020 CPU
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/simulator.h"

/* Forward declarations from CPU core */
extern void cpu_init_state(void);
extern void cpu_execute_opcode(simulator_t *sim);
extern void cpu_execute_many(simulator_t *sim, unsigned long ops);
extern simulator_t *cpu_get_current_simulator(void);
extern void cpu_set_current_simulator(simulator_t *sim);

/* Global CPU structure from cpu_core_new.c */
extern struct tag_CPU {
    struct { long a[8]; } aregs;         /* address registers */
    struct { long d[8]; } dregs;         /* data registers */
    long pc;                              /* program counter */
    struct {
        unsigned sr : 16;                 /* status register */
    } sregs;
    long usp, ssp, msp;                   /* user, supervisor, master stack pointers */
} cpu;

/* Memory access array - maps addresses to modules */
#define MEMORY_MAP_SIZE (16 * 1024)  /* 16K entries = 16MB at 1KB granularity */
static simulator_module_t *memory_map[MEMORY_MAP_SIZE];

/* Global simulator context */
static simulator_t *g_simulator = NULL;

/* ============================================================================
 * Memory Access Functions
 * ============================================================================ */

/**
 * Find module responsible for given address
 */
static simulator_module_t *find_module_for_address(simulator_t *sim, uint32_t addr)
{
    if (sim == NULL) return NULL;

    /* Mask to 24-bit address space */
    addr &= 0xFFFFFF;

    for (int i = 0; i < sim->num_modules; i++) {
        simulator_module_t *mod = sim->modules[i];
        if (addr >= mod->base_addr && addr < (mod->base_addr + mod->size)) {
            return mod;
        }
    }
    return NULL;
}

/**
 * Build memory map from modules for fast address lookup
 */
static void build_memory_map(simulator_t *sim)
{
    memset(memory_map, 0, sizeof(memory_map));

    for (int i = 0; i < sim->num_modules; i++) {
        simulator_module_t *mod = sim->modules[i];
        /* Map each 1KB block to this module */
        uint32_t start_block = mod->base_addr >> 10;
        uint32_t num_blocks = (mod->size + 1023) >> 10;

        for (uint32_t j = 0; j < num_blocks && start_block + j < MEMORY_MAP_SIZE; j++) {
            memory_map[start_block + j] = mod;
        }
    }
}

/**
 * Read from memory via appropriate module
 */
uint32_t simulator_read_memory(simulator_t *sim, uint32_t addr, int size)
{
    if (sim == NULL) return 0;

    addr &= 0xFFFFFF;  /* Mask to 24-bit address space */

    simulator_module_t *mod = find_module_for_address(sim, addr);
    if (mod && mod->read) {
        return mod->read(mod, addr, size);
    }

    /* Bus error - unmappe address */
    fprintf(stderr, "BUS ERROR: Read from unmapped address 0x%06X\n", addr);
    return 0;
}

/**
 * Write to memory via appropriate module
 */
void simulator_write_memory(simulator_t *sim, uint32_t addr, uint32_t data, int size)
{
    if (sim == NULL) return;

    addr &= 0xFFFFFF;  /* Mask to 24-bit address space */

    simulator_module_t *mod = find_module_for_address(sim, addr);
    if (mod && mod->write) {
        mod->write(mod, addr, data, size);
    } else {
        /* Bus error - unmapped address */
        fprintf(stderr, "BUS ERROR: Write to unmapped address 0x%06X\n", addr);
    }
}

/* ============================================================================
 * CPU Execution Loop
 * ============================================================================ */

/**
 * Execute a single CPU instruction
 *
 * Implements the main fetch-decode-execute cycle for MC68020
 */
int simulator_step(simulator_t *sim)
{
    if (sim == NULL) return -1;

    /* Set current simulator context */
    cpu_set_current_simulator(sim);

    /* Execute one CPU opcode (handles fetch, decode, execute) */
    cpu_execute_opcode(sim);

    return 0;
}

/**
 * Execute N CPU instructions
 */
uint32_t simulator_run(simulator_t *sim, uint32_t count)
{
    if (sim == NULL) return 0;

    uint32_t executed = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (simulator_step(sim) != 0) {
            break;  /* Error occurred */
        }
        executed++;
    }

    return executed;
}

/**
 * Pause simulator execution
 */
void simulator_pause(simulator_t *sim)
{
    /* In this simple implementation, pause just stops calling step()
     * More complex implementations might have thread synchronization */
    if (sim) {
        /* No-op for now */
    }
}

/* ============================================================================
 * Initialization and Cleanup
 * ============================================================================ */

/**
 * Initialize the simulator
 */
simulator_t *simulator_init(void)
{
    simulator_t *sim = (simulator_t *)calloc(1, sizeof(simulator_t));
    if (sim == NULL) {
        fprintf(stderr, "Failed to allocate simulator\n");
        return NULL;
    }

    /* Allocate module array (max 64 modules like Windows version) */
    sim->modules = (simulator_module_t **)calloc(64, sizeof(simulator_module_t *));
    if (sim->modules == NULL) {
        fprintf(stderr, "Failed to allocate module array\n");
        free(sim);
        return NULL;
    }
    sim->num_modules = 0;

    /* Initialize CPU state and instruction handlers */
    cpu_init_state();
    cpu_set_current_simulator(sim);

    /* Set as global for access from other modules */
    g_simulator = sim;

    return sim;
}

/**
 * Register a module with the simulator
 */
int simulator_register_module(simulator_t *sim, simulator_module_t *module)
{
    if (sim == NULL || module == NULL) return -1;
    if (sim->num_modules >= 64) {
        fprintf(stderr, "Too many modules (max 64)\n");
        return -1;
    }

    sim->modules[sim->num_modules++] = module;

    /* Sort modules by priority (highest first) */
    for (int i = sim->num_modules - 1; i > 0; i--) {
        if (sim->modules[i]->priority > sim->modules[i-1]->priority) {
            simulator_module_t *tmp = sim->modules[i];
            sim->modules[i] = sim->modules[i-1];
            sim->modules[i-1] = tmp;
        } else {
            break;
        }
    }

    return 0;
}

/**
 * Load all built-in modules
 *
 * This replaces the DLL plugin loading system with static module initialization
 */
int simulator_load_modules(simulator_t *sim)
{
    if (sim == NULL) return -1;

    /* Forward declarations for built-in modules (defined in simulator_modules.c) */
    extern simulator_module_t evmram_module;
    extern simulator_module_t evmrom_module;
    extern simulator_module_t pit68230_module;
    extern simulator_module_t uart68681_module;

    /* Register modules in order */
    if (simulator_register_module(sim, &evmram_module) != 0) {
        fprintf(stderr, "Failed to register RAM module\n");
        return -1;
    }

    if (simulator_register_module(sim, &evmrom_module) != 0) {
        fprintf(stderr, "Failed to register ROM module\n");
        return -1;
    }

    if (simulator_register_module(sim, &pit68230_module) != 0) {
        fprintf(stderr, "Failed to register 68230 PIT module\n");
        return -1;
    }

    if (simulator_register_module(sim, &uart68681_module) != 0) {
        fprintf(stderr, "Failed to register 68681 UART module\n");
        return -1;
    }

    /* Call setup procedures */
    for (int i = 0; i < sim->num_modules; i++) {
        if (sim->modules[i]->setup) {
            if (!sim->modules[i]->setup(sim->modules[i])) {
                fprintf(stderr, "Module %s setup failed\n", sim->modules[i]->name);
                return -1;
            }
        }
    }

    /* Call init procedures (interconnection phase) */
    for (int i = 0; i < sim->num_modules; i++) {
        if (sim->modules[i]->init) {
            sim->modules[i]->init(sim->modules[i]);
        }
    }

    /* Build memory map for fast lookups */
    build_memory_map(sim);

    return 0;
}

/**
 * Reset simulator to initial state
 *
 * Implements MC68020 reset sequence:
 * 1. Read initial SSP from address 0x000000 (longword)
 * 2. Read reset vector from address 0x000004 (longword)
 * 3. Set A7 (SP) = initial SSP
 * 4. Set PC = reset vector
 */
void simulator_reset(simulator_t *sim)
{
    if (sim == NULL) return;

    /* Reset CPU state */
    memset(&sim->cpu, 0, sizeof(simulator_cpu_state_t));
    sim->cpu.sr = 0x2700;  /* Supervisor mode, IPL=7 */

    /* Try to read reset vectors from ROM (0x000000) */
    uint32_t reset_ssp = simulator_read_memory(sim, 0x000000, 4) & 0xFFFFFF;
    uint32_t reset_pc = simulator_read_memory(sim, 0x000004, 4) & 0xFFFFFF;

    /* Validate reset vectors */
    int vectors_valid = 1;
    if (reset_ssp == 0 || reset_pc == 0) {
        fprintf(stderr, "[RESET] WARNING: Zero reset vectors (ROM not loaded?)\n");
        vectors_valid = 0;
    }
    if (reset_pc > 0x00FFFF) {
        /* Reset PC should point to ROM (0x000000-0x00FFFF) */
        fprintf(stderr, "[RESET] WARNING: Reset PC 0x%06X outside ROM range\n", reset_pc);
        vectors_valid = 0;
    }
    if (reset_ssp < 0x400000 || reset_ssp > 0x41FFFF) {
        /* SSP should point to RAM (0x400000-0x41FFFF) */
        fprintf(stderr, "[RESET] WARNING: SSP 0x%06X outside valid RAM range\n", reset_ssp);
        vectors_valid = 0;
    }

    if (!vectors_valid) {
        /* Use default initialization */
        fprintf(stderr, "[RESET] Using default initialization\n");
        sim->cpu.ssp = 0x410000;  /* Supervisor stack in RAM */
        sim->cpu.usp = 0x400000;  /* User stack in RAM */
        sim->cpu.msp = 0x420000;  /* Master stack in RAM */
        sim->cpu.pc = 0x000000;   /* Start at ROM beginning */
    } else {
        /* 68k reset sequence: use validated vectors from ROM */
        sim->cpu.ssp = reset_ssp;
        sim->cpu.usp = 0x400000;  /* User stack (not from vectors) */
        sim->cpu.msp = 0x420000;  /* Master stack (not from vectors) */
        sim->cpu.pc = reset_pc;

        fprintf(stderr, "[RESET] Using 68k reset vectors from ROM:\n");
        fprintf(stderr, "[RESET]   Initial SSP: 0x%06X\n", sim->cpu.ssp);
        fprintf(stderr, "[RESET]   Reset PC:    0x%06X\n", sim->cpu.pc);
    }

    /* Reset all modules */
    for (int i = 0; i < sim->num_modules; i++) {
        if (sim->modules[i]->reset) {
            sim->modules[i]->reset(sim->modules[i]);
        }
    }
}

/**
 * Get current CPU state (read-only)
 */
const simulator_cpu_state_t *simulator_get_state(simulator_t *sim)
{
    if (sim == NULL) return NULL;
    return &sim->cpu;
}

/**
 * Set CPU register value
 */
void simulator_set_register(simulator_t *sim, int reg_id, uint32_t value, char reg_type)
{
    if (sim == NULL || reg_id < 0 || reg_id > 7) return;

    if (reg_type == 'd') {
        sim->cpu.d[reg_id] = value;
    } else if (reg_type == 'a') {
        sim->cpu.a[reg_id] = value;
    }
}

/**
 * Load program into memory
 */
int simulator_load_program(simulator_t *sim, const uint8_t *data, size_t size, uint32_t addr)
{
    if (sim == NULL || data == NULL) return -1;

    for (size_t i = 0; i < size; i++) {
        simulator_write_memory(sim, (addr + i) & 0xFFFFFF, data[i], 1);
    }

    return 0;
}

/**
 * Cleanup and destroy simulator
 */
void simulator_destroy(simulator_t *sim)
{
    if (sim == NULL) return;

    /* Call exit procedures */
    for (int i = 0; i < sim->num_modules; i++) {
        if (sim->modules[i]->exit) {
            sim->modules[i]->exit(sim->modules[i]);
        }
    }

    free(sim->modules);
    free(sim);

    g_simulator = NULL;
}

/**
 * Get global simulator context (for use by modules)
 */
simulator_t *simulator_get_current(void)
{
    return g_simulator;
}
