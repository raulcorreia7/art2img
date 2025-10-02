# Makefile for art2img project
# Simplifies building, testing, and installation using CMake
# Uses built-in FetchContent caching in local _fetch/ directory

BUILD_DIR ?= build
FETCH_DIR ?= _fetch
CMAKE_BUILD_TYPE ?= Release
JOBS ?= $(shell nproc)
CMAKE_ARGS ?= -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DFETCHCONTENT_BASE_DIR=$(abspath $(FETCH_DIR))

.PHONY: all build test clean install debug cache-clean help setup

all: build test

help:
	@echo "Available targets:"
	@echo "  all          - Build and test (default)"
	@echo "  build        - Configure and build project"
	@echo "  test         - Build and run tests"
	@echo "  debug        - Build in Debug mode"
	@echo "  clean        - Remove build directory"
	@echo "  install      - Install to system (requires build)"
	@echo "  cache-clean  - Remove dependency cache (_fetch)"
	@echo "  setup        - Build project"
	@echo "Variables: BUILD_DIR=$(BUILD_DIR), CMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE), JOBS=$(JOBS)"

build:
	@mkdir -p $(BUILD_DIR) $(FETCH_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_ARGS)
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

debug: CMAKE_BUILD_TYPE = Debug
debug: CMAKE_ARGS = -DCMAKE_BUILD_TYPE=Debug -DFETCHCONTENT_BASE_DIR=$(abspath $(FETCH_DIR))
debug: build

test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure -V

clean:
	@rm -rf $(BUILD_DIR)

cache-clean:
	@rm -rf $(FETCH_DIR)

install: build
	@cd $(BUILD_DIR) && cmake --install . --prefix /usr/local

setup: build