#!/bin/sh -e

mkdir -p m4
gtkdocize
autoreconf -i
test -n "$NOCONFIGURE" || ./configure "$@"
