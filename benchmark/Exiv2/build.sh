#!/bin/bash
set -e

cd "$TARGET/repo"
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DEXIV2_ENABLE_INIH=OFF
cmake --build build -j $(nproc)

cp build/bin/exiv2 "$OUT"
