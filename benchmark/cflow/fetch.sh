#!/bin/bash
set -e

if [ -d "$TARGET/repo" ]; then
    exit 0
fi

cd "$TARGET"
wget https://ftp.gnu.org/gnu/cflow/cflow-1.6.tar.gz
tar -xzf cflow-1.6.tar.gz
rm cflow-1.6.tar.gz
mv cflow-1.6 repo
