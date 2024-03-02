#!/bin/bash
set -e

cd "$TARGET/repo"
$CC $CFLAGS -DMJS_MAIN mjs.c -o mjs

cp mjs "$OUT"
