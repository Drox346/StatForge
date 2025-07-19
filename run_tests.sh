#!/bin/bash
set -e
cd "$(dirname "$0")"

cleanup() {
    cd "$OLDPWD"
}
trap cleanup EXIT

export CC=clang
export CXX=clang++

cmake -DTESTING=ON -DEXAMPLES=OFF -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build -S .
cmake --build build
cd build
# ctest --output-on-failure
./tests/test_statforge