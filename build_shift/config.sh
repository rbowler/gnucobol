#!/bin/bash
# Configure gnucobol-shift
CURDIR=$(pwd)
if [ "${CURDIR##*/}" != "gnucobol" ]; then
  echo "configure should be run from the gnucobol directory"
  exit 1
fi
COBVER=$(awk '/Version:/{print$2}' build_shift/control_gnucobol-shift)
./autogen.sh
mkdir -p build
cd build
CFLAGS="-g -rdynamic" ../configure \
 --with-pkgversion=gnucobol-shift \
 --with-bugurl=https://veliashift.com/support \
 --enable-debug \
 --enable-cobc-internal-checks \
 --with-xml2 \
 --with-json \
 --with-curses=no \
 --without-db
 sed -e "/PACKAGE_VERSION/s/3.3-dev/$COBVER/" \
  -e "/PACKAGE_BUGREPORT/s/gnu.org/veliashift.com/" \
  -i config.h
