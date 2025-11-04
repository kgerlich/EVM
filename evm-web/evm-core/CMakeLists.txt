cmake_minimum_required(VERSION 3.15)
project(evm-wasm C)

# Emscripten settings
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
endif()

# Source files from main simulator
set(SIMULATOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

set(SOURCES
    # Core CPU simulator
    "${SIMULATOR_DIR}/EVMSim/Stcom.c"
    "${SIMULATOR_DIR}/EVMSim/stmem.c"
    "${SIMULATOR_DIR}/EVMSim/Stexep.c"
    "${SIMULATOR_DIR}/EVMSim/steacalc.c"
    "${SIMULATOR_DIR}/EVMSim/sttable.c"
    "${SIMULATOR_DIR}/EVMSim/STFLAGS.C"

    # Peripheral modules
    "${SIMULATOR_DIR}/68230/68230.c"
    "${SIMULATOR_DIR}/68230/port.c"
    "${SIMULATOR_DIR}/68230/writes.c"

    "${SIMULATOR_DIR}/68681/68681.c"
    "${SIMULATOR_DIR}/68681/vt100.c"
    "${SIMULATOR_DIR}/68681/state.c"
    "${SIMULATOR_DIR}/68681/printer.c"
    "${SIMULATOR_DIR}/68681/updown.c"

    # Memory modules
    "${SIMULATOR_DIR}/evmram/evmram.c"
    "${SIMULATOR_DIR}/evmrom/evmrom.c"
    "${SIMULATOR_DIR}/evmrom/binary.c"
    "${SIMULATOR_DIR}/evmrom/s19.c"

    # WASM bindings
    "${CMAKE_CURRENT_SOURCE_DIR}/wasm/bindings.c"
)

# Include directories
include_directories(
    "${SIMULATOR_DIR}/EVMSim"
    "${SIMULATOR_DIR}/68230"
    "${SIMULATOR_DIR}/68681"
    "${SIMULATOR_DIR}/evmram"
    "${SIMULATOR_DIR}/evmrom"
)

# Create WASM library
add_executable(evm.js ${SOURCES})

# Emscripten link options
if(EMSCRIPTEN)
    # Use -Oz for better size optimization
    # Allow memory growth for dynamic allocation
    target_link_options(evm.js PRIVATE
        "-sWASM=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_FUNCTIONS=['_cpu_step','_cpu_run','_cpu_reset','_cpu_pause','_cpu_get_state','_cpu_set_register','_cpu_read_memory','_cpu_write_memory','_cpu_load_rom','_cpu_load_program','_malloc','_free']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','getValue','setValue']"
        "-O2"
    )

    # Reduce initial memory
    target_compile_options(evm.js PRIVATE "-Oz")
endif()

# Platform-specific flags
if(UNIX AND NOT APPLE)
    target_compile_options(evm.js PRIVATE -Wno-implicit-function-declaration)
endif()
