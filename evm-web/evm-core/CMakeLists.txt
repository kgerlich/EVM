cmake_minimum_required(VERSION 3.15)
project(evm-wasm C)

# Emscripten settings
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
endif()

# New modular source structure
set(SIMULATOR_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCES
    # New modular simulator core (platform-independent)
    "${SIMULATOR_CORE_DIR}/src/simulator.c"
    "${SIMULATOR_CORE_DIR}/src/simulator_modules.c"

    # Full CPU implementation with instruction handlers
    "${SIMULATOR_CORE_DIR}/src/cpu_core_new.c"
    "${SIMULATOR_CORE_DIR}/src/cpu_instructions.c"
    "${SIMULATOR_CORE_DIR}/src/exception_handlers.c"
    "${SIMULATOR_CORE_DIR}/src/steacalc.c"
    "${SIMULATOR_CORE_DIR}/src/stmem.c"
    "${SIMULATOR_CORE_DIR}/src/sttable.c"

    # WASM bindings
    "${CMAKE_CURRENT_SOURCE_DIR}/wasm/bindings.c"
)

# Include directories for new modular structure
include_directories(
    "${SIMULATOR_CORE_DIR}/include"
)

message(STATUS "Building EVM for: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Sources: ${SOURCES}")

# Create WASM library
add_executable(evm.js ${SOURCES})

# Emscripten link options
if(EMSCRIPTEN)
    # Use -Oz for better size optimization
    # Allow memory growth for dynamic allocation
    target_link_options(evm.js PRIVATE
        "-sWASM=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_FUNCTIONS=['_cpu_init','_cpu_reset','_cpu_shutdown','_cpu_step','_cpu_run','_cpu_pause','_cpu_get_state','_cpu_get_pc','_cpu_set_pc','_cpu_get_dreg','_cpu_set_dreg','_cpu_get_areg','_cpu_set_areg','_cpu_get_sr','_cpu_set_sr','_cpu_read_byte','_cpu_read_word','_cpu_read_dword','_cpu_write_byte','_cpu_write_word','_cpu_write_dword','_cpu_load_program','_cpu_load_rom','_cpu_init_rom','_cpu_is_initialized','_cpu_get_error','_malloc','_free']"
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
