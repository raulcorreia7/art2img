# Simple Makefile for art2image - Linux host with Windows cross-compile

# Linux compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Ivendor -Iinclude
LDFLAGS = -pthread

# Windows cross-compiler
WIN_CXX = x86_64-w64-mingw32-g++
WIN_CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -static -pthread -Ivendor -Iinclude
WIN_LDFLAGS = -static -pthread -lwinpthread

# Source files
SOURCES = src/art_file.cpp src/art2image.cpp src/cli.cpp src/extractor.cpp src/palette.cpp src/png_writer.cpp src/tga_writer.cpp src/threading.cpp
DIAG_SRC = src/diagnostic.cpp

# Default target
all: linux

# Build Linux binaries
linux: bin/art2image bin/art_diagnostic

bin/art2image: $(SOURCES)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOURCES)
	@echo "✅ Built Linux: $@"

bin/art_diagnostic: $(DIAG_SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(DIAG_SRC)
	@echo "✅ Built Linux: $@"

# Build Windows binaries
windows: bin/art2image.exe bin/art_diagnostic.exe

bin/art2image.exe: $(SOURCES)
	@mkdir -p bin
	$(WIN_CXX) $(WIN_CXXFLAGS) $(WIN_LDFLAGS) -o $@ $(SOURCES)
	@echo "✅ Built Windows: $@"

bin/art_diagnostic.exe: $(DIAG_SRC)
	@mkdir -p bin
	$(WIN_CXX) $(WIN_CXXFLAGS) $(WIN_LDFLAGS) -o $@ $(DIAG_SRC)
	@echo "✅ Built Windows: $@"

# Build all platforms
all-platforms: linux windows
	@echo "✅ Built for all platforms"

# Clean
clean:
	rm -rf bin
	@echo "✅ Cleaned build artifacts"

# Test Linux binaries
test: linux
	@mkdir -p tests/output
	./bin/art2image -o tests/output -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART
	@echo "✅ Tests passed"

.PHONY: all linux windows all-platforms clean test