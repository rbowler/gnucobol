#!/bin/bash
# Configure gnucobol-shift
CURDIR=$(pwd)
if [ "${CURDIR##*/}" != "gnucobol" ]; then
  echo "configure should be run from the gnucobol directory"
  exit 1
fi
./autogen.sh
mkdir -p build
cd build
CFLAGS="-g -rdynamic" ../configure \
 --with-pkgversion=gnucobol-shift \
 --enable-debug \
 --enable-cobc-internal-checks \
 --with-xml2 \
 --with-json \
 --with-curses=no \
 --without-db
