#!/bin/bash
# Build gnucobol-shift packages
CURDIR=$(pwd)
if [ "${CURDIR##*/}" != "gnucobol" ]; then
  echo "build should be run from the gnucobol directory"
  exit 1
fi
cd build_shift
./pkg_gnucobol-shift.sh
./pkg_libcob4-shift-dev.sh
./pkg_libcob4t64-shift.sh
echo ""
cd ..
ls -l build/*.deb
