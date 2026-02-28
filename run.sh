#!/bin/bash
set -e

export CC=clang
export CXX=clang++

cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build -S .
cmake --build build
./build/example/example
./build/example/example_c