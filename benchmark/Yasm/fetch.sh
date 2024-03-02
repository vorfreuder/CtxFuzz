#!/bin/bash
set -e

git clone https://github.com/yasm/yasm "$TARGET/repo"
git -C "$TARGET/repo" checkout 9defefa
