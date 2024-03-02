#!/bin/bash
set -e

git clone https://github.com/cisco/openh264 "$TARGET/repo"
git -C "$TARGET/repo" checkout 8684722271ac16118df2fe50322ffe218b9507a7
