/*
 * simulator.h
 *
 * Public API for the EVM MC68020 CPU simulator
 * Platform-independent, works with both Windows (DLL-based) and WASM (static modules)
 */

#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/* MC68020 CPU State */
typedef struct {
    uint16_t sr;                    /* Status Register */
    uint32_t d[8];                  /* Data registers D0-D7 */
    uint32_t a[8];                  /* Address registers A0-A7 */
    uint32_t pc;                    /* Program Counter (24-bit) */
    uint32_t ssp, usp, msp;         /* Stack pointers */
    uint32_t sfc, dfc;              /* Source/Destination function code */
    uint32_t vbr;                   /* Vector base register */
    uint32_t cacr, caar;            /* Cache control */
} simulator_cpu_state_t;

/* Module interface for peripherals (RAM, ROM, 68230, 68681) */
typedef struct simulator_module {
    /* Module info */
    const char *name;
    uint32_t base_addr;             /* Base address in memory map */
    uint32_t size;                  /* Address range size */
    uint16_t priority;              /* Execution priority (higher = execute first) */
    int cacheable;                  /* Whether reads are cacheable */

    /* Function pointers */
    int (*setup)(struct simulator_module *);      /* Initialize module */
    void (*init)(struct simulator_module *);      /* Interconnect phase */
    void (*reset)(struct simulator_module *);     /* Reset to known state */
    void (*exit)(struct simulator_module *);      /* Cleanup */
    void (*simulate)(struct simulator_module *);  /* Called once per CPU cycle */

    /* Memory access */
    uint32_t (*read)(struct simulator_module *, uint32_t addr, int size);
    void (*write)(struct simulator_module *, uint32_t addr, uint32_t data, int size);

    /* Module-specific state pointer */
    void *state;
} simulator_module_t;

/* Main simulator context */
typedef struct {
    simulator_cpu_state_t cpu;      /* CPU state */
    simulator_module_t **modules;   /* Array of loaded modules */
    int num_modules;

    /* Internal state */
    void *priv;                     /* Private simulator data */
} simulator_t;

/* ============================================================================
 * Core Simulator API
 * ============================================================================ */

/**
 * Initialize the simulator
 *
 * Returns: Initialized simulator context, or NULL on error
 */
simulator_t *simulator_init(void);

/**
 * Load all built-in modules (RAM, ROM, 68230, 68681)
 *
 * Must be called after simulator_init() and before simulator_run()
 *
 * Returns: 0 on success, -1 on error
 */
int simulator_load_modules(simulator_t *sim);

/**
 * Reset simulator to initial state
 */
void simulator_reset(simulator_t *sim);

/**
 * Execute N CPU instructions
 *
 * count: Number of instructions to execute
 * Returns: Number of instructions actually executed
 */
uint32_t simulator_run(simulator_t *sim, uint32_t count);

/**
 * Execute a single CPU instruction
 *
 * Returns: 0 on success, non-zero on error
 */
int simulator_step(simulator_t *sim);

/**
 * Pause simulator execution
 */
void simulator_pause(simulator_t *sim);

/**
 * Get current CPU state
 */
const simulator_cpu_state_t *simulator_get_state(simulator_t *sim);

/**
 * Set CPU register value
 *
 * register_id: Register number (0-7 for D/A registers)
 * value: Value to set
 * reg_type: 'd' for data register, 'a' for address register
 */
void simulator_set_register(simulator_t *sim, int register_id, uint32_t value, char reg_type);

/**
 * Read from memory
 *
 * addr: Address to read from
 * size: Number of bytes (1, 2, 4)
 * Returns: Value read from memory
 */
uint32_t simulator_read_memory(simulator_t *sim, uint32_t addr, int size);

/**
 * Write to memory
 *
 * addr: Address to write to
 * data: Value to write
 * size: Number of bytes (1, 2, 4)
 */
void simulator_write_memory(simulator_t *sim, uint32_t addr, uint32_t data, int size);

/**
 * Load ROM/program image
 *
 * data: Pointer to program data
 * size: Size of data in bytes
 * addr: Address to load at (typically 0x000000 for ROM or 0x400000 for RAM)
 * Returns: 0 on success, -1 on error
 */
int simulator_load_program(simulator_t *sim, const uint8_t *data, size_t size, uint32_t addr);

/**
 * Cleanup and destroy simulator
 */
void simulator_destroy(simulator_t *sim);

/* ============================================================================
 * Module Registration (for static module initialization)
 * ============================================================================ */

/**
 * Register a built-in module
 *
 * Used during simulator initialization to add modules like RAM, ROM, 68230, 68681
 */
int simulator_register_module(simulator_t *sim, simulator_module_t *module);

/**
 * Get module at given address
 *
 * Returns: Module handle, or NULL if no module at address
 */
simulator_module_t *simulator_get_module_at(simulator_t *sim, uint32_t addr);

/**
 * Get the current global simulator context
 *
 * Used internally by modules and CPU core to access the current simulator
 */
simulator_t *simulator_get_current(void);

/* ============================================================================
 * Memory Access Functions (called by CPU core and exception handlers)
 * ============================================================================ */

/**
 * Read byte from memory
 */
uint8_t cpu_read_byte(uint32_t addr);

/**
 * Read word (16-bit) from memory
 */
uint16_t cpu_read_word(uint32_t addr);

/**
 * Read dword (32-bit) from memory
 */
uint32_t cpu_read_dword(uint32_t addr);

/**
 * Write byte to memory
 */
void cpu_write_byte(uint32_t addr, uint8_t value);

/**
 * Write word (16-bit) to memory
 */
void cpu_write_word(uint32_t addr, uint16_t value);

/**
 * Write dword (32-bit) to memory
 */
void cpu_write_dword(uint32_t addr, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* __SIMULATOR_H__ */
