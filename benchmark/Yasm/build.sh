#!/bin/bash
set -e

cd "$TARGET/repo"
export ASAN_OPTIONS=detect_leaks=0
./autogen.sh
./configure
make -j $(nproc)

cp yasm "$OUT"
