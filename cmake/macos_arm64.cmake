set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)

# Cross-compilation tools (osxcross)
set(CMAKE_C_COMPILER oa64-clang)
set(CMAKE_CXX_COMPILER oa64-clang++)

# Target architecture
set(CMAKE_OSX_ARCHITECTURES arm64)

# Static linking for standalone binaries
set(CMAKE_CXX_FLAGS "-static")
set(CMAKE_EXE_LINKER_FLAGS "-static")