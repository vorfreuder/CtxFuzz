#!/bin/bash
set -e

cd "$TARGET/repo"
export ASAN_OPTIONS=detect_leaks=0 # Centos7 needs
./configure --disable-shared
make -j $(nproc)

cp binutils/cxxfilt "$OUT"
