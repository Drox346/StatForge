#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

cleanup() { cd "$OLDPWD"; }
trap cleanup EXIT

export CC=clang
export CXX=clang++

# Build: keep portable + fast, but also good callstacks for perf.
# (Frame pointers help perf -g. Still no -march/-mtune.)
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

# Run once normally (so you can see your own timings/output)
./tests/test_statforge

# If perf is blocked on your system, you may need:
#   sudo sysctl -w kernel.perf_event_paranoid=1
# (or 0 if still blocked)

# Sampling profile (hot functions + call stacks)
perf record -g -- ./tests/test_statforge
perf report

# Extra counters (cache misses, branches, IPC-ish signals)
perf stat -d -d -- ./tests/test_statforge
