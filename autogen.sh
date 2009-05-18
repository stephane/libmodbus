#! /bin/sh
autoreconf -v --force --install || exit 1
./configure "$@"
