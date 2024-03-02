#!/bin/bash
set -e

cd "$TARGET/repo"
make -j $(nproc)

cp gif2rgb "$OUT"
