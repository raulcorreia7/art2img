# Generic cross-compilation toolchain template
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/generic.cmake

# Set system name and processor (override these when calling cmake)
if(NOT CMAKE_SYSTEM_NAME)
    message(FATAL_ERROR "CMAKE_SYSTEM_NAME must be set (e.g., Windows, Linux, Darwin)")
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR)
    message(FATAL_ERROR "CMAKE_SYSTEM_PROCESSOR must be set (e.g., x86_64, arm64)")
endif()

# Use environment variables for compiler selection
# Set these before running cmake:
# export CMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
# export CMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++

# Standard cross-compilation settings
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Static linking for standalone binaries (optional)
if(DEFINED STATIC_LINKING AND STATIC_LINKING)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(CMAKE_SHARED_LINKER_FLAGS "-static")
endif()