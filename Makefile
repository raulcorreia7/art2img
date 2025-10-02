# Makefile for art2img
# Simplified build system for x86/x64 Linux and Windows

# Project information
PROJECT_NAME = art2img
VERSION = 1.0.0

# Compiler configuration
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -pthread -Ivendor -Iinclude
LDFLAGS ?= -pthread

# Windows cross-compiler (x86_64 only)
WIN_CXX ?= x86_64-w64-mingw32-g++
WIN_CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -static -pthread -Ivendor -Iinclude
WIN_LDFLAGS ?= -static -pthread -lwinpthread

# Detect number of processors for parallel builds
NPROCS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

# Directories
SRCDIR = src
BINDIR = bin
OBJDIR = obj
LIBDIR = lib
TESTDIR = tests

# Source files
MAIN_SOURCES = $(wildcard $(SRCDIR)/*.cpp)
MAIN_SOURCES := $(filter-out $(SRCDIR)/diagnostic.cpp $(SRCDIR)/test_%.cpp, $(MAIN_SOURCES))
DIAG_SOURCES = $(SRCDIR)/diagnostic.cpp

# Object files
MAIN_OBJECTS = $(MAIN_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DIAG_OBJECTS = $(DIAG_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Windows-specific directories
WIN_OBJDIR = $(OBJDIR)-win
WIN_MAIN_OBJECTS = $(MAIN_SOURCES:$(SRCDIR)/%.cpp=$(WIN_OBJDIR)/%.o)
WIN_DIAG_OBJECTS = $(DIAG_SOURCES:$(SRCDIR)/%.cpp=$(WIN_OBJDIR)/%.o)

# Default target
all: linux

# Platform targets
linux: $(BINDIR)/$(PROJECT_NAME) $(BINDIR)/$(PROJECT_NAME)_diagnostic

windows: 
	@echo "Building Windows binaries with $(NPROCS) parallel jobs..."
	@$(MAKE) -j$(NPROCS) _windows

_windows: $(BINDIR)/$(PROJECT_NAME).exe $(BINDIR)/$(PROJECT_NAME)_diagnostic.exe

# Linux build rules
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BINDIR)/$(PROJECT_NAME): $(MAIN_OBJECTS)
	@echo "Building Linux $(PROJECT_NAME)..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

$(BINDIR)/$(PROJECT_NAME)_diagnostic: $(DIAG_OBJECTS)
	@echo "Building Linux diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

# Windows build rules
$(WIN_OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(WIN_OBJDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -c -o $@ $<

$(BINDIR)/$(PROJECT_NAME).exe: $(WIN_MAIN_OBJECTS)
	@echo "Building Windows $(PROJECT_NAME)..."
	@mkdir -p $(BINDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $^ $(WIN_LDFLAGS)
	@echo "Built: $@"

$(BINDIR)/$(PROJECT_NAME)_diagnostic.exe: $(WIN_DIAG_OBJECTS)
	@echo "Building Windows diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $^ $(WIN_LDFLAGS)
	@echo "Built: $@"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
	rm -rf $(WIN_OBJDIR)
	rm -rf $(LIBDIR)
	rm -rf $(TESTDIR)/output/*
	@echo "Clean complete"

# Test
test: linux
	@echo "Running functionality tests..."
	./$(BINDIR)/$(PROJECT_NAME) -o $(TESTDIR)/output/tga -f tga -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/$(PROJECT_NAME) -o $(TESTDIR)/output/png -f png -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/$(PROJECT_NAME) -o $(TESTDIR)/output/with_transparency -f png -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	./$(BINDIR)/$(PROJECT_NAME) -o $(TESTDIR)/output/no_transparency -f png -p $(TESTDIR)/assets/PALETTE.DAT -N $(TESTDIR)/assets/TILES000.ART
	@cd $(TESTDIR) && ./test_functionality.sh
	@echo "Tests completed successfully"

# Verify binary architectures
verify: linux
	@echo "Verifying binary architectures..."
	@file $(BINDIR)/$(PROJECT_NAME) 2>/dev/null | grep -q "ELF" && echo "✓ Linux $(PROJECT_NAME): ELF binary" || echo "✗ Linux $(PROJECT_NAME): Wrong architecture"
	@file $(BINDIR)/$(PROJECT_NAME)_diagnostic 2>/dev/null | grep -q "ELF" && echo "✓ Linux $(PROJECT_NAME)_diagnostic: ELF binary" || echo "✗ Linux $(PROJECT_NAME)_diagnostic: Wrong architecture"
	@if command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then \
		$(MAKE) _windows >/dev/null 2>&1; \
		file $(BINDIR)/$(PROJECT_NAME).exe 2>/dev/null | grep -q "PE32+" && echo "✓ Windows $(PROJECT_NAME).exe: PE binary" || echo "✗ Windows $(PROJECT_NAME).exe: Wrong architecture"; \
		file $(BINDIR)/$(PROJECT_NAME)_diagnostic.exe 2>/dev/null | grep -q "PE32+" && echo "✓ Windows $(PROJECT_NAME)_diagnostic.exe: PE binary" || echo "✗ Windows $(PROJECT_NAME)_diagnostic.exe: Wrong architecture"; \
	else \
		echo "⚠ Windows cross-compiler not available, skipping Windows verification"; \
	fi

# Show detected processor count
procs:
	@echo "Detected $(NPROCS) processors"

# Phony targets
.PHONY: all linux windows _windows clean test verify procs