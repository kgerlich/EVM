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
    /* Port A */
    uint8_t PADR;       /* Port A Data Register */
    uint8_t PADDR;      /* Port A Direction Register */

    /* Port B */
    uint8_t PBDR;       /* Port B Data Register */
    uint8_t PBDDR;      /* Port B Direction Register */

    /* Port C */
    uint8_t PCDR;       /* Port C Data Register */
    uint8_t PCDDR;      /* Port C Direction Register */

    /* Control Registers */
    uint8_t PGCR;       /* Port General Control Register */
    uint8_t PSRR;       /* Port Service Request Register */
    uint8_t PACR;       /* Port A Control Register */
    uint8_t PBCR;       /* Port B Control Register */
    uint8_t PAAR;       /* Port A Alternate Register */
    uint8_t PBAR;       /* Port B Alternate Register */
    uint8_t PSR;        /* Port Status Register */

    /* Timer Registers */
    uint8_t TCR;        /* Timer Control Register */
    uint8_t TIVR;       /* Timer Interrupt Vector Register */
    uint8_t TSR;        /* Timer Status Register */
    uint8_t PIVR;       /* Port Interrupt Vector Register */

    /* Counter Registers */
    uint8_t CPRH, CPRM, CPRL;  /* Counter Preload */
    uint8_t CNTRH, CNTRM, CNTRL;  /* Counter */

    /* Timer state */
    uint32_t counter;
    uint32_t preload;
    uint32_t tick_count;

} pit_state_t;

#define PIT_BASE_ADDR   0x800000
#define PIT_SIZE        0x36

static pit_state_t pit_state = {0};

static int pit_setup(simulator_module_t *mod)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    memset(state, 0, sizeof(pit_state_t));
    state->preload = 0xFFFFFF;
    state->counter = 0xFFFFFF;
    return 1;  /* Success */
}

static void pit_init(simulator_module_t *mod)
{
    /* No special initialization */
}

static void pit_reset(simulator_module_t *mod)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    memset(state, 0, sizeof(pit_state_t));
    state->preload = 0xFFFFFF;
    state->counter = 0xFFFFFF;
}

static void pit_exit(simulator_module_t *mod)
{
    /* No cleanup needed */
}

static void pit_simulate(simulator_module_t *mod)
{
    pit_state_t *state = (pit_state_t *)mod->state;

    /* Basic timer decrement - decrements every 1000 ticks */
    if ((++(state->tick_count) & 0x3FF) == 0) {
        if (state->counter > 0) {
            state->counter--;
        } else {
            /* Timer underflow */
            state->counter = state->preload;
            state->TSR |= 0x01;  /* Set underflow bit */
        }
    }
}

static uint32_t pit_read(simulator_module_t *mod, uint32_t addr, int size)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0x3F;

    if (size == 1) {
        /* Register mapping based on offset */
        switch (offset) {
            case 0x01: return state->PGCR;
            case 0x03: return state->PSRR;
            case 0x05: return state->PADDR;
            case 0x07: return state->PBDDR;
            case 0x09: return state->PCDDR;
            case 0x0B: return state->PIVR;
            case 0x0D: return state->PACR;
            case 0x0F: return state->PBCR;
            case 0x11: return state->PADR;
            case 0x13: return state->PBDR;
            case 0x15: return state->PAAR;
            case 0x17: return state->PBAR;
            case 0x19: return state->PCDR;
            case 0x1B: return state->PSR;
            case 0x21: return state->TCR;
            case 0x23: return state->TIVR;
            case 0x27: return state->CNTRH;
            case 0x29: return state->CNTRM;
            case 0x2B: return state->CNTRL;
            case 0x2D: return state->TSR;
            default: return 0xFF;
        }
    } else if (size == 2) {
        uint32_t result = pit_read(mod, addr, 1);
        result = (result << 8);
        if (offset + 1 < PIT_SIZE) {
            result |= pit_read(mod, addr + 1, 1);
        }
        return result;
    }
    return 0;
}

static void pit_write(simulator_module_t *mod, uint32_t addr, uint32_t data, int size)
{
    pit_state_t *state = (pit_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0x3F;

    if (size == 1) {
        data = data & 0xFF;
        switch (offset) {
            case 0x01: state->PGCR = data; break;
            case 0x03: state->PSRR = data; break;
            case 0x05: state->PADDR = data; break;
            case 0x07: state->PBDDR = data; break;
            case 0x09: state->PCDDR = data; break;
            case 0x0B: state->PIVR = data & 0xFC; break;
            case 0x0D: state->PACR = data; break;
            case 0x0F: state->PBCR = data; break;
            case 0x11: state->PADR = data & state->PADDR; break;  /* Mask by direction */
            case 0x13: state->PBDR = data & state->PBDDR; break;
            case 0x15: state->PAAR = data; break;
            case 0x17: state->PBAR = data; break;
            case 0x19: state->PCDR = data & state->PCDDR; break;
            case 0x21: state->TCR = data; break;
            case 0x23: state->TIVR = data; break;
            case 0x25: state->CPRH = data; break;
            case 0x27: state->CPRM = data; break;
            case 0x29: state->CPRL = data; break;
            case 0x2B: state->CNTRH = data; break;
            case 0x2D: state->CNTRM = data; break;
            case 0x2F: state->CNTRL = data; break;
            case 0x31: state->TSR = 0; break;  /* Writing clears flags */
        }
    } else if (size == 2) {
        pit_write(mod, addr, (data >> 8) & 0xFF, 1);
        if (offset + 1 < PIT_SIZE) {
            pit_write(mod, addr + 1, data & 0xFF, 1);
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
 * 68681 UART Module (Dual UART at 0xA00000)
 * ============================================================================ */

typedef struct {
    /* Channel A */
    uint8_t MR1A;       /* Mode Register 1A */
    uint8_t SRA;        /* Status Register A */
    uint8_t CSRA;       /* Clock Select Register A */
    uint8_t CRA;        /* Command Register A */
    uint8_t RBA;        /* Receiver Buffer A */
    uint8_t TBA;        /* Transmitter Buffer A */
    uint8_t IPCR;       /* Input Port Change Register */
    uint8_t IPU;        /* Input Port Unlatched */

    /* Channel B */
    uint8_t MR1B;       /* Mode Register 1B */
    uint8_t SRB;        /* Status Register B */
    uint8_t CSRB;       /* Clock Select Register B */
    uint8_t CRB;        /* Command Register B */
    uint8_t RBB;        /* Receiver Buffer B */
    uint8_t TBB;        /* Transmitter Buffer B */

    /* Common Registers */
    uint8_t ACR;        /* Auxiliary Control Register */
    uint8_t ISR;        /* Interrupt Status Register */
    uint8_t IMR;        /* Interrupt Mask Register */
    uint8_t IVR;        /* Interrupt Vector Register */
    uint8_t CTUR;       /* Counter/Timer Upper Register */
    uint8_t CTLR;       /* Counter/Timer Lower Register */
    uint8_t CMSB;       /* Current MSB of Counter */
    uint8_t CLSB;       /* Current LSB of Counter */
    uint8_t OPCR;       /* Output Port Configuration */
    uint8_t OPR;        /* Output Port Register */

    /* State tracking */
    uint16_t tx_buffer_a;
    uint16_t tx_buffer_b;
    uint32_t counter;
    uint32_t rx_ready_a;  /* Fake RX for testing */
    uint32_t rx_ready_b;

} uart_state_t;

#define UART_BASE_ADDR  0xA00000
#define UART_SIZE       0x20

static uart_state_t uart_state = {0};

static int uart_setup(simulator_module_t *mod)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    memset(state, 0, sizeof(uart_state_t));
    /* Initialize status registers with TXRDY bits set (ready to transmit) */
    state->SRA = 0x04;  /* TXRDY for channel A */
    state->SRB = 0x04;  /* TXRDY for channel B */
    return 1;  /* Success */
}

static void uart_init(simulator_module_t *mod)
{
    /* No special initialization */
}

static void uart_reset(simulator_module_t *mod)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    memset(state, 0, sizeof(uart_state_t));
    state->SRA = 0x04;
    state->SRB = 0x04;
}

static void uart_exit(simulator_module_t *mod)
{
    /* No cleanup needed */
}

static void uart_simulate(simulator_module_t *mod)
{
    uart_state_t *state = (uart_state_t *)mod->state;

    /* Simple simulation: always set TXRDY and periodically set RXRDY */
    state->SRA |= 0x04;  /* TXRDY - transmitter ready */
    state->SRB |= 0x04;  /* TXRDY - transmitter ready */

    /* Simulate occasional RX data availability */
    if ((++state->counter & 0x1FFF) == 0) {
        state->SRA |= 0x01;  /* Set RXRDY for channel A */
        state->rx_ready_a = 1;
    }
}

static uint32_t uart_read(simulator_module_t *mod, uint32_t addr, int size)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0x1F;

    if (size == 1) {
        /* Register mapping for 68681 DUART */
        switch (offset) {
            /* Channel A */
            case 0x01: return state->MR1A;
            case 0x03: return state->SRA;
            case 0x05: return 0xFF;  /* Reserved */
            case 0x07:
                state->SRA &= ~0x01;  /* Clear RXRDY after read */
                return state->RBA;
            case 0x09: return state->IPCR;
            case 0x0B: return state->ISR;
            case 0x0D: return state->CMSB;
            case 0x0F: return state->CLSB;

            /* Channel B */
            case 0x11: return state->MR1B;
            case 0x13: return state->SRB;
            case 0x15: return 0xFF;  /* Reserved */
            case 0x17:
                state->SRB &= ~0x01;  /* Clear RXRDY after read */
                return state->RBB;
            case 0x19: return state->IVR;
            case 0x1B: return state->IPU;
            case 0x1D: return 0xFF;  /* Reserved */
            case 0x1F: return 0xFF;  /* Reserved */

            default: return 0xFF;
        }
    }
    return 0;
}

static void uart_write(simulator_module_t *mod, uint32_t addr, uint32_t data, int size)
{
    uart_state_t *state = (uart_state_t *)mod->state;
    uint32_t offset = (addr - mod->base_addr) & 0x1F;

    if (size == 1) {
        data = data & 0xFF;
        switch (offset) {
            /* Channel A */
            case 0x01: state->MR1A = data; break;
            case 0x03: state->CSRA = data; break;
            case 0x05: state->CRA = data; break;
            case 0x07:
                state->TBA = data;
                /* Echo output to stderr for debugging */
                if (data >= 32 && data < 127) {
                    fprintf(stderr, "%c", data);
                } else if (data == '\n') {
                    fprintf(stderr, "\n");
                }
                fflush(stderr);
                state->SRA |= 0x04;  /* Set TXRDY */
                break;
            case 0x09: state->ACR = data; break;
            case 0x0B: state->IMR = data; break;
            case 0x0D: state->CTUR = data; break;
            case 0x0F: state->CTLR = data; break;

            /* Channel B */
            case 0x11: state->MR1B = data; break;
            case 0x13: state->CSRB = data; break;
            case 0x15: state->CRB = data; break;
            case 0x17:
                state->TBB = data;
                /* Echo output */
                if (data >= 32 && data < 127) {
                    fprintf(stderr, "%c", data);
                } else if (data == '\n') {
                    fprintf(stderr, "\n");
                }
                fflush(stderr);
                state->SRB |= 0x04;  /* Set TXRDY */
                break;
            case 0x19: state->IVR = data; break;
            case 0x1B: state->OPCR = data; break;
            case 0x1D: state->OPR = data; break;  /* Set output port */
            case 0x1F: state->OPR &= ~data; break;  /* Clear output port */
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
