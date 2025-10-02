#!/bin/bash
cmake -S . -B build -DBUILD_TESTS=ON 
cmake --build build 
ctest --test-dir build --output-on-failure