# Makefile for art2image
# Container-optimized build system

# Project information
PROJECT_NAME = art2image
VERSION = 1.0.0

# Compiler configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Ivendor -Iinclude
LDFLAGS = -pthread

# Windows cross-compiler
WIN_CXX = x86_64-w64-mingw32-g++
WIN_CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -static -pthread -Ivendor -Iinclude
WIN_LDFLAGS = -static -pthread -lwinpthread

# Directories
SRCDIR = src
BINDIR = bin
TESTDIR = tests

# Source files
MAIN_SOURCES = src/art_file.cpp src/art2image.cpp src/cli.cpp src/extractor.cpp src/palette.cpp src/png_writer.cpp src/tga_writer.cpp src/threading.cpp
DIAG_SOURCES = src/diagnostic.cpp

# Ensure test script is executable
$(TESTDIR)/test_functionality.sh:
	@chmod +x $(TESTDIR)/test_functionality.sh

# Default target
all: linux

# Linux binaries
linux: $(BINDIR)/art2image $(BINDIR)/art_diagnostic

# Windows binaries (cross-compile)
windows: $(BINDIR)/art2image.exe $(BINDIR)/art_diagnostic.exe

# Main executable (Linux)
$(BINDIR)/art2image: $(MAIN_SOURCES)
	@echo "Building art2image executable..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_SOURCES) $(LDFLAGS)
	@echo "Built: $@"

# Diagnostic tool (Linux)
$(BINDIR)/art_diagnostic: $(DIAG_SOURCES)
	@echo "Building diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(DIAG_SOURCES) $(LDFLAGS)
	@echo "Built: $@"

# Windows executable
$(BINDIR)/art2image.exe: $(MAIN_SOURCES)
	@echo "Building Windows art2image executable..."
	@mkdir -p $(BINDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $(MAIN_SOURCES) $(WIN_LDFLAGS)
	@echo "Built: $@"

# Windows diagnostic tool
$(BINDIR)/art_diagnostic.exe: $(DIAG_SOURCES)
	@echo "Building Windows diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $(DIAG_SOURCES) $(WIN_LDFLAGS)
	@echo "Built: $@"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BINDIR)
	rm -rf $(TESTDIR)/output/*
	@echo "Clean complete"

# Test
test: linux $(TESTDIR)/test_functionality.sh
	@echo "Running functionality tests..."
	./$(BINDIR)/art2image -o $(TESTDIR)/output/tga -f tga -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/art2image -o $(TESTDIR)/output/png -f png -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/art2image -o $(TESTDIR)/output/with_transparency -f png -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/art2image -o $(TESTDIR)/output/no_transparency -f png -p $(TESTDIR)/assets/PALETTE.DAT -N $(TESTDIR)/assets/TILES000.ART
	@./$(TESTDIR)/test_functionality.sh
	@echo "Tests completed successfully"

# Verify binary architectures
verify: linux windows
	@echo "Verifying binary architectures..."
	@file $(BINDIR)/art2image 2>/dev/null | grep -q "ELF" && echo "✓ Linux art2image: ELF binary" || echo "✗ Linux art2image: Wrong architecture"
	@file $(BINDIR)/art_diagnostic 2>/dev/null | grep -q "ELF" && echo "✓ Linux art_diagnostic: ELF binary" || echo "✗ Linux art_diagnostic: Wrong architecture"
	@file $(BINDIR)/art2image.exe 2>/dev/null | grep -q "PE32+" && echo "✓ Windows art2image.exe: PE binary" || echo "✗ Windows art2image.exe: Wrong architecture"
	@file $(BINDIR)/art_diagnostic.exe 2>/dev/null | grep -q "PE32+" && echo "✓ Windows art_diagnostic.exe: PE binary" || echo "✗ Windows art_diagnostic.exe: Wrong architecture"

.PHONY: all linux windows clean test verify
