#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

cleanup() { cd "$OLDPWD"; }
trap cleanup EXIT

export CC=clang
export CXX=clang++

common_cflags="-O3 -DNDEBUG -g -fno-omit-frame-pointer"
common_cxxflags="$common_cflags"
common_ldflags="-flto"

cmake \
  -DTESTING=ON \
  -DEXAMPLES=OFF \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_C_FLAGS_RELWITHDEBINFO="$common_cflags" \
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="$common_cxxflags" \
  -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="$common_ldflags" \
  -DCMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO="$common_ldflags" \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
  -B build -S .

cmake --build build
cd build

./tests/benchmark_statforge

perf record -g -- ./tests/benchmark_statforge
perf report

perf stat -d -d -- ./tests/benchmark_statforge
