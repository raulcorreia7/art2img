set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Cross-compilation tools
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# Target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# Modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Static linking for standalone binaries
set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")