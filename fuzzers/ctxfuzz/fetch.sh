#!/bin/bash
set -e

if [ -d "$FUZZER/repo" ]; then
    exit 0
fi
