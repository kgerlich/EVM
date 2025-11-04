/*
 * simulator_modules.c
 *
 * Static module implementations for WASM build
 * Provides RAM, ROM, 68230 PIT, and 68681 UART modules
 *
 * This is a simplified version suitable for WASM.
 * The Windows build can use the full DLL-based modules.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/simulator.h"

/* ============================================================================
 * RAM Module (128KB at 0x400000)
 * ============================================================================ */

typedef struct {
    uint8_t *memory;
    size_t size;
} ram_state_t;

#define RAM_BASE_ADDR   0x400000
#define RAM_SIZE        (128 * 1024)

static ram_state_t ram_state = {0};

static int ram_setup(simulator_module_t *mod)
{
    ram_state_t *state = (ram_state_t *)mod->state;
    state->size = RAM_SIZE;
    state->memory = (uint8_t *)malloc(RAM_SIZE);
    if (state->memory == NULL) {
        fprintf(stderr, "Failed to allocate RAM\n");
        return 0;
    }
    memset(state->memory, 0, RAM_SIZE);
    return 1;  /* Success */
}

static void ram_init(simulator_module_t *mod)
{
    /* No special initialization needed */
}

static void ram_reset(simulator_module_t *mod)
{
    ram_state_t *state = (ram_state_t *)mod->state;
    memset(state->memory, 0, state->size);
}

static void ram_exit(simulator_module_t *mod)
{
    ram_state_t *state = (ram_state_t *)mod->state;
    if (state->memory) {
        free(state->memory);
        state->memory = NULL;
    }
}

static void ram_simulate(simulator_module_t *mod)
{
    /* No simulation needed for passive memory */
}

static uint32_t ram_read(simulator_module_t *mod, uint32_t addr, int size)
{
    ram_state_t *state = (ram_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFFFFFF;

    if (offset + size > state->size) {
        fprintf(stderr, "RAM: Out of bounds read at 0x%06X size %d\n", addr, size);
        return 0;
    }

    uint32_t result = 0;
    if (size == 1) {
        result = state->memory[offset];
    } else if (size == 2) {
        /* Big-endian word */
        result = ((uint32_t)state->memory[offset] << 8) |
                  (uint32_t)state->memory[offset + 1];
    } else if (size == 4) {
        /* Big-endian dword */
        result = ((uint32_t)state->memory[offset] << 24) |
                 ((uint32_t)state->memory[offset + 1] << 16) |
                 ((uint32_t)state->memory[offset + 2] << 8) |
                  (uint32_t)state->memory[offset + 3];
    }
    return result;
}

static void ram_write(simulator_module_t *mod, uint32_t addr, uint32_t data, int size)
{
    ram_state_t *state = (ram_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFFFFFF;

    if (offset + size > state->size) {
        fprintf(stderr, "RAM: Out of bounds write at 0x%06X size %d\n", addr, size);
        return;
    }

    if (size == 1) {
        state->memory[offset] = (uint8_t)data;
    } else if (size == 2) {
        /* Big-endian word */
        state->memory[offset] = (uint8_t)((data >> 8) & 0xFF);
        state->memory[offset + 1] = (uint8_t)(data & 0xFF);
    } else if (size == 4) {
        /* Big-endian dword */
        state->memory[offset] = (uint8_t)((data >> 24) & 0xFF);
        state->memory[offset + 1] = (uint8_t)((data >> 16) & 0xFF);
        state->memory[offset + 2] = (uint8_t)((data >> 8) & 0xFF);
        state->memory[offset + 3] = (uint8_t)(data & 0xFF);
    }
}

simulator_module_t evmram_module = {
    .name = "EVMRAM",
    .base_addr = RAM_BASE_ADDR,
    .size = RAM_SIZE,
    .priority = 50,  /* Normal priority */
    .cacheable = 1,
    .setup = ram_setup,
    .init = ram_init,
    .reset = ram_reset,
    .exit = ram_exit,
    .simulate = ram_simulate,
    .read = ram_read,
    .write = ram_write,
    .state = &ram_state
};

/* ============================================================================
 * Boot RAM Module (64KB at 0x000000)
 *
 * Handles the lower 64KB of memory (0x000000-0x00FFFF) where the boot ROM
 * image is loaded. This is read-write RAM, not read-only ROM, allowing
 * the simulator to load the S19 ROM image into this region.
 * ============================================================================ */

typedef struct {
    uint8_t *memory;
    size_t size;
} rom_state_t;

#define ROM_BASE_ADDR   0x000000
#define ROM_SIZE        (64 * 1024)

static rom_state_t rom_state = {0};

static int rom_setup(simulator_module_t *mod)
{
    rom_state_t *state = (rom_state_t *)mod->state;
    state->size = ROM_SIZE;
    state->memory = (uint8_t *)malloc(ROM_SIZE);
    if (state->memory == NULL) {
        fprintf(stderr, "Failed to allocate ROM\n");
        return 0;
    }
    memset(state->memory, 0, ROM_SIZE);
    return 1;  /* Success */
}

static void rom_init(simulator_module_t *mod)
{
    /* No special initialization */
}

static void rom_reset(simulator_module_t *mod)
{
    /* ROM is not reset */
}

static void rom_exit(simulator_module_t *mod)
{
    rom_state_t *state = (rom_state_t *)mod->state;
    if (state->memory) {
        free(state->memory);
        state->memory = NULL;
    }
}

static void rom_simulate(simulator_module_t *mod)
{
    /* No simulation needed */
}

static uint32_t rom_read(simulator_module_t *mod, uint32_t addr, int size)
{
    rom_state_t *state = (rom_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFFFFFF;

    if (offset + size > state->size) {
        fprintf(stderr, "ROM: Out of bounds read at 0x%06X size %d\n", addr, size);
        return 0;
    }

    uint32_t result = 0;
    if (size == 1) {
        result = state->memory[offset];
    } else if (size == 2) {
        /* Big-endian word */
        result = ((uint32_t)state->memory[offset] << 8) |
                  (uint32_t)state->memory[offset + 1];
    } else if (size == 4) {
        /* Big-endian dword */
        result = ((uint32_t)state->memory[offset] << 24) |
                 ((uint32_t)state->memory[offset + 1] << 16) |
                 ((uint32_t)state->memory[offset + 2] << 8) |
                  (uint32_t)state->memory[offset + 3];
    }
    return result;
}

static void rom_write(simulator_module_t *mod, uint32_t addr, uint32_t data, int size)
{
    rom_state_t *state = (rom_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFFFFFF;

    if (offset + size > state->size) {
        fprintf(stderr, "ROM: Out of bounds write at 0x%06X size %d\n", addr, size);
        return;
    }

    if (size == 1) {
        state->memory[offset] = (uint8_t)data;
    } else if (size == 2) {
        /* Big-endian word */
        state->memory[offset] = (uint8_t)((data >> 8) & 0xFF);
        state->memory[offset + 1] = (uint8_t)(data & 0xFF);
    } else if (size == 4) {
        /* Big-endian dword */
        state->memory[offset] = (uint8_t)((data >> 24) & 0xFF);
        state->memory[offset + 1] = (uint8_t)((data >> 16) & 0xFF);
        state->memory[offset + 2] = (uint8_t)((data >> 8) & 0xFF);
        state->memory[offset + 3] = (uint8_t)(data & 0xFF);
    }
}

simulator_module_t evmrom_module = {
    .name = "EVMROM",
    .base_addr = ROM_BASE_ADDR,
    .size = ROM_SIZE,
    .priority = 100,  /* Highest priority */
    .cacheable = 1,
    .setup = rom_setup,
    .init = rom_init,
    .reset = rom_reset,
    .exit = rom_exit,
    .simulate = rom_simulate,
    .read = rom_read,
    .write = rom_write,
    .state = &rom_state
};

/* ============================================================================
 * 68230 PIT Module (Parallel Interface/Timer at 0x800000)
 * ============================================================================ */

typedef struct {
    uint8_t registers[256];
} pit_state_t;

#define PIT_BASE_ADDR   0x800000
#define PIT_SIZE        256

static pit_state_t pit_state = {0};

static int pit_setup(simulator_module_t *mod)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    memset(state->registers, 0, sizeof(state->registers));
    return 1;  /* Success */
}

static void pit_init(simulator_module_t *mod)
{
    /* No special initialization */
}

static void pit_reset(simulator_module_t *mod)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    memset(state->registers, 0, sizeof(state->registers));
}

static void pit_exit(simulator_module_t *mod)
{
    /* No cleanup needed */
}

static void pit_simulate(simulator_module_t *mod)
{
    /* Timer simulation would go here */
}

static uint32_t pit_read(simulator_module_t *mod, uint32_t addr, int size)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFF;

    if (size == 1) {
        return state->registers[offset];
    } else if (size == 2) {
        uint32_t result = ((uint32_t)state->registers[offset] << 8);
        if (offset + 1 < sizeof(state->registers)) {
            result |= state->registers[offset + 1];
        }
        return result;
    }
    return 0;
}

static void pit_write(simulator_module_t *mod, uint32_t addr, uint32_t data, int size)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFF;

    if (size == 1) {
        state->registers[offset] = (uint8_t)data;
    } else if (size == 2) {
        state->registers[offset] = (uint8_t)((data >> 8) & 0xFF);
        if (offset + 1 < sizeof(state->registers)) {
            state->registers[offset + 1] = (uint8_t)(data & 0xFF);
        }
    }
}

simulator_module_t pit68230_module = {
    .name = "68230 PIT",
    .base_addr = PIT_BASE_ADDR,
    .size = PIT_SIZE,
    .priority = 50,  /* Normal priority */
    .cacheable = 0,
    .setup = pit_setup,
    .init = pit_init,
    .reset = pit_reset,
    .exit = pit_exit,
    .simulate = pit_simulate,
    .read = pit_read,
    .write = pit_write,
    .state = &pit_state
};

/* ============================================================================
 * 68681 UART Module (at 0xA00000)
 * ============================================================================ */

typedef struct {
    uint8_t registers[256];
} uart_state_t;

#define UART_BASE_ADDR  0xA00000
#define UART_SIZE       256

static uart_state_t uart_state = {0};

static int uart_setup(simulator_module_t *mod)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    memset(state->registers, 0, sizeof(state->registers));
    return 1;  /* Success */
}

static void uart_init(simulator_module_t *mod)
{
    /* No special initialization */
}

static void uart_reset(simulator_module_t *mod)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    memset(state->registers, 0, sizeof(state->registers));
}

static void uart_exit(simulator_module_t *mod)
{
    /* No cleanup needed */
}

static void uart_simulate(simulator_module_t *mod)
{
    /* UART simulation would go here */
}

static uint32_t uart_read(simulator_module_t *mod, uint32_t addr, int size)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFF;

    if (size == 1) {
        return state->registers[offset];
    }
    return 0;
}

static void uart_write(simulator_module_t *mod, uint32_t addr, uint32_t data, int size)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0xFF;

    if (size == 1) {
        state->registers[offset] = (uint8_t)data;

        /* Simple UART output - write to stdout */
        if (offset == 0) {  /* TX buffer */
            putchar((int)data);
            fflush(stdout);
        }
    }
}

simulator_module_t uart68681_module = {
    .name = "68681 UART",
    .base_addr = UART_BASE_ADDR,
    .size = UART_SIZE,
    .priority = 50,  /* Normal priority */
    .cacheable = 0,
    .setup = uart_setup,
    .init = uart_init,
    .reset = uart_reset,
    .exit = uart_exit,
    .simulate = uart_simulate,
    .read = uart_read,
    .write = uart_write,
    .state = &uart_state
};
