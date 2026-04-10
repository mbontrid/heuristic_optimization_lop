cmake_minimum_required(VERSION 3.10...4.1)

project(heuristic_optimization_lop C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Automatically collect all C source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Include directory
include_directories(src)

# Base/Release target with maximum optimization and -DNDEBUG
add_executable(lop ${SOURCES})
find_package(OpenMP REQUIRED)
target_link_libraries(lop PRIVATE OpenMP::OpenMP_C)
target_compile_options(lop PRIVATE -Ofast -Wall -fopenmp-simd -march=native)
# target_compile_options(lop PRIVATE -Ofast -Wall -fopenmp -march=native -fopt-info-vec-missed -fopt-info-vec)
target_compile_definitions(lop PRIVATE NDEBUG)

# Debug target with -Og and no -DNDEBUG
add_executable(lop_debug ${SOURCES})
target_compile_options(lop_debug PRIVATE -Og -Wall)

# Set output directory
set_target_properties(lop lop_debug PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Custom target to build both
add_custom_target(all_targets ALL DEPENDS lop lop_debug)
