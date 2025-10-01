# Makefile for art2img
# Container-optimized build system

# Project information
PROJECT_NAME = art2img
VERSION = 1.0.0

# Compiler configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Ivendor -Iinclude
LDFLAGS = -pthread

# Windows cross-compiler
WIN_CXX = x86_64-w64-mingw32-g++
WIN_CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -static -pthread -Ivendor -Iinclude
WIN_LDFLAGS = -static -pthread -lwinpthread

# Linux ARM64 cross-compiler
LINUX_ARM64_CXX = aarch64-linux-gnu-g++
LINUX_ARM64_CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Ivendor -Iinclude
LINUX_ARM64_LDFLAGS = -pthread

# Windows ARM64 cross-compiler
WIN_ARM64_CXX = aarch64-w64-mingw32-g++
WIN_ARM64_CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -static -pthread -Ivendor -Iinclude
WIN_ARM64_LDFLAGS = -static -pthread -lwinpthread

# Directories
SRCDIR = src
BINDIR = bin
OBJDIR = obj
LIBDIR = lib
TESTDIR = tests

# Source files
MAIN_SOURCES = src/art_file.cpp src/art2img.cpp src/cli.cpp src/extractor.cpp src/palette.cpp src/png_writer.cpp src/tga_writer.cpp src/threading.cpp
DIAG_SOURCES = src/diagnostic.cpp
LIB_SOURCES = src/art_file.cpp src/extractor.cpp src/palette.cpp src/png_writer.cpp src/tga_writer.cpp src/extractor_api.cpp src/threading.cpp

# Object files
MAIN_OBJECTS = $(MAIN_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DIAG_OBJECTS = $(DIAG_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
LIB_OBJECTS = $(LIB_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Ensure test script is executable
$(TESTDIR)/test_functionality.sh:
	@chmod +x $(TESTDIR)/test_functionality.sh

# Default target
all: linux library

# Linux binaries
linux: $(BINDIR)/art2img $(BINDIR)/art_diagnostic

# Library targets
library: $(LIBDIR)/libart2img.a $(LIBDIR)/libart2img.so

# Windows binaries (cross-compile)
windows: $(BINDIR)/art2img.exe $(BINDIR)/art_diagnostic.exe

# Linux ARM64 binaries (cross-compile)
linux-arm64: $(BINDIR)/art2img-arm64 $(BINDIR)/art_diagnostic-arm64

# Windows ARM64 binaries (cross-compile)
windows-arm64: $(BINDIR)/art2img-arm64.exe $(BINDIR)/art_diagnostic-arm64.exe

# Main executable (Linux)
$(BINDIR)/art2img: $(MAIN_OBJECTS)
	@echo "Building art2img executable..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

# Diagnostic tool (Linux)
$(BINDIR)/art_diagnostic: $(DIAG_OBJECTS)
	@echo "Building diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

# Static library
$(LIBDIR)/libart2img.a: $(LIB_OBJECTS)
	@echo "Building static library..."
	@mkdir -p $(LIBDIR)
	ar rcs $@ $^
	@echo "Built: $@"

# Shared library
$(LIBDIR)/libart2img.so: $(LIB_OBJECTS)
	@echo "Building shared library..."
	@mkdir -p $(LIBDIR)
	$(CXX) -shared -fPIC $(CXXFLAGS) -o $@ $^
	@echo "Built: $@"

# Object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Windows executable
$(BINDIR)/art2img.exe: $(MAIN_SOURCES)
	@echo "Building Windows art2img executable..."
	@mkdir -p $(BINDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $(MAIN_SOURCES) $(WIN_LDFLAGS)
	@echo "Built: $@"

# Windows diagnostic tool
$(BINDIR)/art_diagnostic.exe: $(DIAG_SOURCES)
	@echo "Building Windows diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $(DIAG_SOURCES) $(WIN_LDFLAGS)
	@echo "Built: $@"

# Linux ARM64 executable
$(BINDIR)/art2img-arm64: $(MAIN_SOURCES)
	@echo "Building Linux ARM64 art2img executable..."
	@mkdir -p $(BINDIR)
	$(LINUX_ARM64_CXX) $(LINUX_ARM64_CXXFLAGS) -o $@ $(MAIN_SOURCES) $(LINUX_ARM64_LDFLAGS)
	@echo "Built: $@"

# Linux ARM64 diagnostic tool
$(BINDIR)/art_diagnostic-arm64: $(DIAG_SOURCES)
	@echo "Building Linux ARM64 diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(LINUX_ARM64_CXX) $(LINUX_ARM64_CXXFLAGS) -o $@ $(DIAG_SOURCES) $(LINUX_ARM64_LDFLAGS)
	@echo "Built: $@"

# Windows ARM64 executable
$(BINDIR)/art2img-arm64.exe: $(MAIN_SOURCES)
	@echo "Building Windows ARM64 art2img executable..."
	@mkdir -p $(BINDIR)
	$(WIN_ARM64_CXX) $(WIN_ARM64_CXXFLAGS) -o $@ $(MAIN_SOURCES) $(WIN_ARM64_LDFLAGS)
	@echo "Built: $@"

# Windows ARM64 diagnostic tool
$(BINDIR)/art_diagnostic-arm64.exe: $(DIAG_SOURCES)
	@echo "Building Windows ARM64 diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(WIN_ARM64_CXX) $(WIN_ARM64_CXXFLAGS) -o $@ $(DIAG_SOURCES) $(WIN_ARM64_LDFLAGS)
	@echo "Built: $@"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
	rm -rf $(LIBDIR)
	rm -rf $(TESTDIR)/output/*
	@echo "Clean complete"

# Test
test: linux $(TESTDIR)/test_functionality.sh
	@echo "Running functionality tests..."
	./$(BINDIR)/art2img -o $(TESTDIR)/output/tga -f tga -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/art2img -o $(TESTDIR)/output/png -f png -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/art2img -o $(TESTDIR)/output/with_transparency -f png -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/art2img -o $(TESTDIR)/output/no_transparency -f png -p $(TESTDIR)/assets/PALETTE.DAT -N $(TESTDIR)/assets/TILES000.ART
	@./$(TESTDIR)/test_functionality.sh
	@echo "Tests completed successfully"

# Library test
test-library: library
	@echo "Building library test..."
	@mkdir -p $(TESTDIR)/output
	$(CXX) $(CXXFLAGS) -o $(TESTDIR)/output/test_library tests/test_library_api.cpp -L$(LIBDIR) -lart2img $(LDFLAGS)
	@echo "Running library test..."
	$(TESTDIR)/output/test_library
	@echo "Library test completed successfully"

# Verify binary architectures
verify: linux
	@echo "Verifying binary architectures..."
	@file $(BINDIR)/art2img 2>/dev/null | grep -q "ELF" && echo "✓ Linux art2img: ELF binary" || echo "✗ Linux art2img: Wrong architecture"
	@file $(BINDIR)/art_diagnostic 2>/dev/null | grep -q "ELF" && echo "✓ Linux art_diagnostic: ELF binary" || echo "✗ Linux art_diagnostic: Wrong architecture"
	@if command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then \
		$(MAKE) windows >/dev/null 2>&1; \
		file $(BINDIR)/art2img.exe 2>/dev/null | grep -q "PE32+" && echo "✓ Windows art2img.exe: PE binary" || echo "✗ Windows art2img.exe: Wrong architecture"; \
		file $(BINDIR)/art_diagnostic.exe 2>/dev/null | grep -q "PE32+" && echo "✓ Windows art_diagnostic.exe: PE binary" || echo "✗ Windows art_diagnostic.exe: Wrong architecture"; \
	else \
		echo "⚠ Windows cross-compiler not available, skipping Windows verification"; \
	fi

# Verify Linux ARM64 binaries
verify-linux-arm64:
	@if command -v aarch64-linux-gnu-g++ >/dev/null 2>&1; then \
		$(MAKE) linux-arm64 >/dev/null 2>&1; \
		echo "Verifying Linux ARM64 binaries..."; \
		file $(BINDIR)/art2img-arm64 2>/dev/null | grep -q "ELF.*aarch64" && echo "✓ Linux ARM64 art2img: ELF aarch64 binary" || echo "✗ Linux ARM64 art2img: Wrong architecture"; \
		file $(BINDIR)/art_diagnostic-arm64 2>/dev/null | grep -q "ELF.*aarch64" && echo "✓ Linux ARM64 art_diagnostic: ELF aarch64 binary" || echo "✗ Linux ARM64 art_diagnostic: Wrong architecture"; \
	else \
		echo "⚠ Linux ARM64 cross-compiler not available, skipping ARM64 verification"; \
	fi

# Verify Windows ARM64 binaries
verify-windows-arm64:
	@if command -v aarch64-w64-mingw32-g++ >/dev/null 2>&1; then \
		$(MAKE) windows-arm64 >/dev/null 2>&1; \
		echo "Verifying Windows ARM64 binaries..."; \
		file $(BINDIR)/art2img-arm64.exe 2>/dev/null | grep -q "PE32.*ARM64" && echo "✓ Windows ARM64 art2img.exe: PE ARM64 binary" || echo "✗ Windows ARM64 art2img.exe: Wrong architecture"; \
		file $(BINDIR)/art_diagnostic-arm64.exe 2>/dev/null | grep -q "PE32.*ARM64" && echo "✓ Windows ARM64 art_diagnostic.exe: PE ARM64 binary" || echo "✗ Windows ARM64 art_diagnostic.exe: Wrong architecture"; \
	else \
		echo "⚠ Windows ARM64 cross-compiler not available, skipping Windows ARM64 verification"; \
	fi

# Verify all platforms
verify-all: linux windows linux-arm64 windows-arm64
	@echo "Verifying all binary architectures..."
	@$(MAKE) verify
	@$(MAKE) verify-linux-arm64
	@$(MAKE) verify-windows-arm64
	@echo "All binary architectures verified!"

# Build all platforms
build-all: linux windows linux-arm64 windows-arm64
	@echo "Building all platform binaries..."
	@echo "Build complete for all platforms!"

.PHONY: all linux windows linux-arm64 windows-arm64 clean test verify verify-linux-arm64 verify-windows-arm64 verify-all build-all library
