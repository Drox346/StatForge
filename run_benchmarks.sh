#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

cleanup() { cd "$OLDPWD"; }
trap cleanup EXIT

export CC=clang
export CXX=clang++

# Portable "benchmark-style" opts:
# -O3 + NDEBUG + LTO, but NO -march/-mtune native.
# Keep it fair for distributed binaries.
common_cflags="-O3 -DNDEBUG -fomit-frame-pointer"
common_cxxflags="$common_cflags"
common_ldflags="-flto"

cmake \
  -DTESTING=ON \
  -DEXAMPLES=OFF \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_C_FLAGS_RELEASE="$common_cflags" \
  -DCMAKE_CXX_FLAGS_RELEASE="$common_cxxflags" \
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="$common_ldflags" \
  -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="$common_ldflags" \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
  -B build -S .

cmake --build build --config Release
cd build
./tests/test_statforge