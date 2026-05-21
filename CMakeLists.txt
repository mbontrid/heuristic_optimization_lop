cmake_minimum_required(VERSION 3.10...4.1)

project(heuristic_optimization_lop C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Automatically collect all C source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Include directory
include_directories(src)

find_package(OpenMP REQUIRED)
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

# Debug target with -Og and no -DNDEBUG
add_executable(lop_debug ${SOURCES})
target_link_libraries(lop_debug PRIVATE OpenMP::OpenMP_C)
target_compile_options(lop_debug PRIVATE -Og -Wall)
target_compile_definitions(lop_debug PRIVATE DEBUG)

# Base/Release target with maximum optimization and -DNDEBUG
add_executable(lop ${SOURCES})
target_link_libraries(lop PRIVATE OpenMP::OpenMP_C)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(lop PRIVATE -Ofast -Wall -fopenmp-simd -march=native -mtune=native -fomit-frame-pointer -funroll-loops)
else()
  target_compile_options(lop PRIVATE -Ofast -Wall -fopenmp-simd -march=native)
endif()
target_compile_definitions(lop PRIVATE NDEBUG)
if(IPO_SUPPORTED)
  set_property(TARGET lop PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO/LTO not supported: ${IPO_ERROR}")
endif()

# Set output directory
set_target_properties(lop lop_debug PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Custom target to build both
add_custom_target(all_targets ALL DEPENDS lop lop_debug)
