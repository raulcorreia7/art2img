# Windows cross-compilation toolchain file
# This toolchain file is used for cross-compiling to Windows using MinGW
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Target environment location
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Adjust the default behavior of the FIND_XXX() commands:
# Search for programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set compiler flags for static linking
# Use -static for static linking and -static-libgcc -static-libstdc++ for static libraries
# For MinGW, we use -mwindows for GUI applications and -mconsole for console applications
set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_C_FLAGS "-static -static-libgcc")

# Set linker flags for static linking
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-static -static-libgcc -static-libstdc++ -flto=auto")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-static -static-libgcc -static-libstdc++ -flto=auto")

# Set thread library for MinGW
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 1)
set(CMAKE_USE_PTHREADS_INIT 0)

# Override output directories to keep Windows builds separate
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set default build type to Release
set(CMAKE_BUILD_TYPE_INIT "Release")

# Set optimization flags for Release build
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG -flto=auto -ffast-math")
set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG -flto=auto -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0")
set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0")

# Remove problematic flags that might be set
# Ensure we don't have any conflicting flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE_INIT}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG_INIT}")

# Define Windows specific macros
add_definitions(-DWIN32)
add_definitions(-D_WINDOWS)
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-D_WIN32_WINNT=0x0601)