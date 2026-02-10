#!/bin/bash
set -e

./configure --quiet
make clean --quiet
make CFLAGS="--coverage -O0" LDFLAGS="--coverage" --quiet
make coverage || true
open coverage/index.html 2>/dev/null || xdg-open coverage/index.html 2>/dev/null || echo "Report: coverage/index.html"
