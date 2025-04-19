#!/bin/bash
# Date: 2016/09/07
# Version: v3.5.0

set -e

if [ -d "$TARGET/repo" ]; then
    exit 0
fi

git clone https://github.com/VirusTotal/yara "$TARGET/repo"
git -C "$TARGET/repo" checkout v3.5.0 # 2016/09/07
