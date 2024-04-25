#!/bin/bash
set -e

cd "$TARGET"
wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz # 2023/07/30
tar -xzf binutils-2.41.tar.gz
rm binutils-2.41.tar.gz
mv binutils-2.41 repo
