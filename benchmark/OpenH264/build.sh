#!/bin/bash
set -e

cd "$TARGET/repo"
make -j $(nproc)

cp h264dec "$OUT"
