# Makefile for art2img project
# Simplifies building, testing, and installation using CMake
# Uses CPM (CMake Package Manager) for dependency management

BUILD_DIR ?= build
CMAKE_BUILD_TYPE ?= Release
JOBS ?= $(shell nproc)
CMAKE_ARGS ?= -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Sanitizer options

.PHONY: all build test clean install debug help setup format format-check asan-build leak-build test-asan test-leak

all: build test

help:
	@echo "Available targets:"
	@echo "  all          - Build and test (default)"
	@echo "  build        - Configure and build project"
	@echo "  test         - Build and run tests"
	@echo "  debug        - Build in Debug mode"
	@echo "  clean        - Remove build directory"
	@echo "  install      - Install to system (requires build)"
	@echo "  setup        - Build project"
	@echo "  format       - Format source code with clang-format"
	@echo "  format-check - Check if source code is properly formatted"
	@echo "  asan-build   - Build with AddressSanitizer"
	@echo "  leak-build   - Build with LeakSanitizer"
	@echo "  test-asan    - Run tests with AddressSanitizer"
	@echo "  test-leak    - Run tests with LeakSanitizer"
	@echo "Variables: BUILD_DIR=$(BUILD_DIR), CMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE), JOBS=$(JOBS)"

build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_ARGS)
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

debug: CMAKE_BUILD_TYPE = Debug
debug: CMAKE_ARGS = -DCMAKE_BUILD_TYPE=Debug
debug: build

test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure -V

clean:
	@rm -rf $(BUILD_DIR)

install: build
	@cd $(BUILD_DIR) && cmake --install . --prefix /usr/local

format:
	@./format_code.sh

format-check:
	@echo "Checking code formatting..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_ARGS)
	@cmake --build $(BUILD_DIR) --target clang-format-dry-run
	@echo "Code formatting check complete!"

# AddressSanitizer build
asan-build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_ARGS) -DENABLE_ASAN=ON
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

# LeakSanitizer build
leak-build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_ARGS) -DENABLE_LEAK_SANITIZER=ON
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

# Test with AddressSanitizer
test-asan: asan-build
	@cd $(BUILD_DIR) && ctest --output-on-failure -V

# Test with LeakSanitizer
test-leak: leak-build
	@cd $(BUILD_DIR) && ctest --output-on-failure -V