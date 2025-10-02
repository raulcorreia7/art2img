# art2img build orchestration -------------------------------------------------
# Keep the project buildable with the stock toolchain while remaining friendly
# to people who override flags or compilers via the environment.

PROJECT        := art2img
VERSION        := 1.0.0

SHELL          := /bin/bash
MAKEFLAGS      += --warn-undefined-variables
MAKEFLAGS      += --no-builtin-rules

# Detect number of processors for parallel compilation
NPROCS         := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

.SUFFIXES:

# ---- Toolchain -----------------------------------------------------------------
CXX            ?= g++
CPPFLAGS       ?=
CPPFLAGS       += -Ivendor -Iinclude
CXXFLAGS       ?= -std=c++17 -Wall -Wextra -O2 -pthread -pipe
LDFLAGS        ?= -pthread
DEPFLAGS       := -MMD -MP

# Use all available processors for parallel compilation
MAKEFLAGS      += -j$(NPROCS)

WIN_CXX        ?= x86_64-w64-mingw32-g++
WIN_CPPFLAGS   ?= $(CPPFLAGS)
WIN_CXXFLAGS   ?= -std=c++17 -Wall -Wextra -O2 -static -pthread -pipe
WIN_LDFLAGS    ?= -static -pthread -lwinpthread

RM             ?= rm -f
RMDIR          ?= rm -rf

# ---- Layout --------------------------------------------------------------------
SRCDIR         := src
BINDIR         := bin
OBJDIR         := obj
WIN_OBJDIR     := obj-win
TESTDIR        := tests
OUTPUT_DIR     := $(TESTDIR)/output
OUTPUT_VARIANTS := tga png with_transparency no_transparency
ASSET_DIR      := $(TESTDIR)/assets

ART_ASSET      := $(ASSET_DIR)/TILES000.ART
PALETTE_ASSET  := $(ASSET_DIR)/PALETTE.DAT

# Source inventory (diagnostic builds from a single file, everything else is main)
DIAG_SOURCE    := $(SRCDIR)/diagnostic.cpp
COMMON_SOURCES := $(filter-out $(DIAG_SOURCE) $(wildcard $(SRCDIR)/test_%.cpp),                     $(wildcard $(SRCDIR)/*.cpp))

# Linux artefacts ----------------------------------------------------------------
PROGRAM        := $(BINDIR)/$(PROJECT)
PROGRAM_DIAG   := $(BINDIR)/$(PROJECT)_diagnostic
LINUX_BINS     := $(PROGRAM) $(PROGRAM_DIAG)

PROGRAM_OBJECTS      := $(COMMON_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
PROGRAM_DIAG_OBJECT  := $(DIAG_SOURCE:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Windows artefacts --------------------------------------------------------------
WIN_PROGRAM          := $(PROGRAM).exe
WIN_PROGRAM_DIAG     := $(PROGRAM_DIAG).exe
WINDOWS_BINS         := $(WIN_PROGRAM) $(WIN_PROGRAM_DIAG)

WIN_PROGRAM_OBJECTS      := $(COMMON_SOURCES:$(SRCDIR)/%.cpp=$(WIN_OBJDIR)/%.o)
WIN_PROGRAM_DIAG_OBJECT  := $(DIAG_SOURCE:$(SRCDIR)/%.cpp=$(WIN_OBJDIR)/%.o)

# Dependency files generated via -MMD/-MP
LINUX_DEPS     := $(PROGRAM_OBJECTS:.o=.d) $(PROGRAM_DIAG_OBJECT:.o=.d)
WINDOWS_DEPS   := $(WIN_PROGRAM_OBJECTS:.o=.d) $(WIN_PROGRAM_DIAG_OBJECT:.o=.d)

# Default goal -------------------------------------------------------------------
.DEFAULT_GOAL := linux

# Directory helpers --------------------------------------------------------------
$(BINDIR) $(OBJDIR) $(WIN_OBJDIR):
	@mkdir -p $@

# Compilation rules --------------------------------------------------------------
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

$(WIN_OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(WIN_OBJDIR)
	$(WIN_CXX) $(WIN_CPPFLAGS) $(WIN_CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

# Linking rules ------------------------------------------------------------------
$(PROGRAM): $(PROGRAM_OBJECTS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM_DIAG): $(PROGRAM_DIAG_OBJECT) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(WIN_PROGRAM): $(WIN_PROGRAM_OBJECTS) | $(BINDIR) check-win-toolchain
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $^ $(WIN_LDFLAGS)

$(WIN_PROGRAM_DIAG): $(WIN_PROGRAM_DIAG_OBJECT) | $(BINDIR) check-win-toolchain
	$(WIN_CXX) $(WIN_CXXFLAGS) -o $@ $^ $(WIN_LDFLAGS)

# Platform aggregates ------------------------------------------------------------
all: linux

linux: $(LINUX_BINS)
	@echo "Linux binaries ready: $(LINUX_BINS)"

check-win-toolchain:
	@command -v $(WIN_CXX) >/dev/null 2>&1 || { \
		echo "✗ $(WIN_CXX) not available"; \
		exit 1; \
	}

windows-build: check-win-toolchain $(WINDOWS_BINS)

windows: check-win-toolchain windows-build
	@echo "Windows binaries ready: $(WINDOWS_BINS)"

# Convenience -------------------------------------------------------------------
clean:
	@echo "Cleaning build artifacts..."
	-$(RMDIR) $(BINDIR) $(OBJDIR) $(WIN_OBJDIR)
	-$(RMDIR) $(OUTPUT_DIR)
	@echo "Clean complete"

procs:
	@echo "Detected $$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1) processors"

# Tests -------------------------------------------------------------------------
test: $(PROGRAM)
	@echo "Running functionality tests..."
	@mkdir -p $(addprefix $(OUTPUT_DIR)/,$(OUTPUT_VARIANTS))
	$(PROGRAM) -o $(OUTPUT_DIR)/tga -f tga -p $(PALETTE_ASSET) $(ART_ASSET)
	$(PROGRAM) -o $(OUTPUT_DIR)/png -f png -p $(PALETTE_ASSET) $(ART_ASSET)
	$(PROGRAM) -o $(OUTPUT_DIR)/with_transparency -f png -p $(PALETTE_ASSET) $(ART_ASSET)
	$(PROGRAM) -o $(OUTPUT_DIR)/no_transparency -f png -p $(PALETTE_ASSET) -N $(ART_ASSET)
	@cd $(TESTDIR) && ./test_functionality.sh
	@echo "Tests completed successfully"

help:
	@echo "Available targets:"
	@printf "  %-20s%s\\n" "linux" "Build Linux binaries"
	@printf "  %-20s%s\\n" "windows" "Build Windows binaries (if toolchain present)"
	@printf "  %-20s%s\\n" "test" "Run smoke tests"
	@printf "  %-20s%s\\n" "verify" "Check binary formats"
	@printf "  %-20s%s\\n" "clean" "Remove build artefacts"

# Verification ------------------------------------------------------------------
verify: linux
	@echo "Verifying binary architectures..."
	@$(MAKE) --no-print-directory verify-linux
	@$(MAKE) --no-print-directory verify-windows
	@echo "Verification complete."

verify-linux:
	@echo "- Linux binaries"
	@for bin in $(LINUX_BINS); do \
		label=$$(basename $$bin); \
		if [ ! -f $$bin ]; then \
			echo "  ✗ $$label: Missing binary"; \
		elif file $$bin 2>/dev/null | grep -q "ELF"; then \
			echo "  ✓ $$label: ELF binary"; \
		else \
			echo "  ✗ $$label: Wrong architecture"; \
		fi; \
	done

verify-windows:
	@if command -v $(WIN_CXX) >/dev/null 2>&1; then \
		$(MAKE) --no-print-directory windows-build; \
		echo "- Windows binaries"; \
		for bin in $(WINDOWS_BINS); do \
			label=$$(basename $$bin); \
			if file $$bin 2>/dev/null | grep -q "PE32+"; then \
				echo "  ✓ $$label: PE binary"; \
			else \
				echo "  ✗ $$label: Wrong architecture"; \
			fi; \
			done; \
	else \
		echo "⚠ $(WIN_CXX) not available, skipping Windows verification"; \
	fi

# Dependency includes -----------------------------------------------------------
-include $(LINUX_DEPS) $(WINDOWS_DEPS)

# Phony rules -------------------------------------------------------------------
.PHONY: all linux windows windows-build clean test verify verify-linux verify-windows procs check-win-toolchain help
