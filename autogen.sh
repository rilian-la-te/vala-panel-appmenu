#!/bin/sh -e

mkdir -p m4
autoreconf -i
test -n "$NOCONFIGURE" || ./configure "$@"
