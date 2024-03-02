#!/bin/bash
set -e

git clone https://github.com/VirusTotal/yara "$TARGET/repo"
git -C "$TARGET/repo" checkout v3.5.0
