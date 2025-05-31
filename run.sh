#!/bin/bash
set -e

cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build -S .
cmake --build build
./build/example/example