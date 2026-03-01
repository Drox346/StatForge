#!/bin/bash
set -e

export CC=clang
export CXX=clang++

cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build -S .
cmake --build build
./build/example/example
./build/example/example_c
if command -v luajit >/dev/null 2>&1; then
  luajit example/lua/example.lua
else
  echo "Skipping Lua example: luajit not found"
fi
if command -v go >/dev/null 2>&1; then
  GOCACHE=/tmp/go-build-cache LD_LIBRARY_PATH=build/src go run example/golang/example.go
else
  echo "Skipping Go example: go not found"
fi
