#!/bin/sh -e
# SPDX-License_identifier: AGPL-3.0-only

if [ x"$CC" = x ]; then
    CC=$(which clang)
fi
if [ x"$CC" = x ]; then
    CC=$(which gcc)
fi
if [ x"$CC" = x ]; then
    printf "Need a C compiler (project prefers clang)\n"
    exit 1
fi

CFLAGS="-O2 -fstack-protector-strong -Wformat -Werror=format-security $CFLAGS"
CPPFLAGS="-Wdate-time -D_FORTIFY_SOURCE=2 $CPPFLAGS"
LDFLAGS="-Wl,-z,relro $LDFLAGS"

set -x
$CC -std=c11 -Wall -Wextra -Werror -g $CPPFLAGS $CFLAGS $LDFLAGS \
    -o main main.c buf.c
exec ./main
