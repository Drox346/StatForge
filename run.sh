#!/bin/bash
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -Bbuild && cd build && ninja && ./example/example ; cd "$(dirname "$0")"