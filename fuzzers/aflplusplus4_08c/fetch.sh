#!/bin/bash
set -e

if [ -d "$FUZZER/repo" ]; then
    exit 0
fi

git clone https://github.com/AFLplusplus/AFLplusplus "$FUZZER/repo"
git -C "$FUZZER/repo" checkout v4.08c
