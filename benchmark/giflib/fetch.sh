#!/bin/bash
# https://giflib.sourceforge.net/
set -e

git clone https://git.code.sf.net/p/giflib/code "$TARGET/repo"
git -C "$TARGET/repo" checkout 5.2.1
