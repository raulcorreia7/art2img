# Makefile for art2image
# Container-optimized build system

# Project information
PROJECT_NAME = art2image
VERSION = 1.0.0

# Compiler configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Ivendor -Iinclude
LDFLAGS = -pthread

# Directories
SRCDIR = src
BINDIR = bin
TESTDIR = tests

# Source files
MAIN_SOURCES = src/art_file.cpp src/art2image.cpp src/cli.cpp src/extractor.cpp src/palette.cpp src/png_writer.cpp src/tga_writer.cpp src/threading.cpp
DIAG_SOURCES = src/diagnostic.cpp

# Default target
all: $(BINDIR)/art2image $(BINDIR)/art_diagnostic

# Main executable
$(BINDIR)/art2image: $(MAIN_SOURCES)
	@echo "Building art2image executable..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_SOURCES) $(LDFLAGS)
	@echo "Built: $@"

# Diagnostic tool
$(BINDIR)/art_diagnostic: $(DIAG_SOURCES)
	@echo "Building diagnostic tool..."
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(DIAG_SOURCES) $(LDFLAGS)
	@echo "Built: $@"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BINDIR)
	@echo "Clean complete"

# Test
test: all
	@echo "Running functionality tests..."
	@mkdir -p $(TESTDIR)/output
	./$(BINDIR)/art2image -o $(TESTDIR)/output -p $(TESTDIR)/assets/PALETTE.DAT $(TESTDIR)/assets/TILES000.ART
	@echo "Tests completed successfully"

.PHONY: all clean test