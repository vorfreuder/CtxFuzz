#!/bin/bash
set -e

git clone https://github.com/Exiv2/exiv2 "$TARGET/repo"
git -C "$TARGET/repo" checkout v0.26
