#!/bin/bash
set -e

git clone https://github.com/cesanta/mjs "$TARGET/repo"
git -C "$TARGET/repo" checkout b1b6eac
