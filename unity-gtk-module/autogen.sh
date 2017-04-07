#!/bin/sh -e

mkdir -p m4
gtkdocize --copy
autoreconf -i
test -n "$NOCONFIGURE" || ./configure "$@"
