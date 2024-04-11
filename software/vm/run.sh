#!/usr/bin/bash

set -euo pipefail
./as2.py $1 out.bin
cmake --build build
./build/pc_port/pc_port out.bin