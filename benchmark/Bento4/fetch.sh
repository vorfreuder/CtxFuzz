#!/bin/bash
set -e

git clone https://github.com/axiomatic-systems/Bento4 "$TARGET/repo"
git -C "$TARGET/repo" checkout v1.6.0-639
