#!/bin/sh
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
CPPFLAGS="-Wdate-time -D_FORTIFY_SOURCE=2 $CFLAGS"
LDFLAGS="-Wl,-z,relro $LDFLAGS"

set -x
$CC -std=c11 -Wall -Wextra -Werror $CPPFLAGS $CFLAGS $LDFLAGS -o main main.c
