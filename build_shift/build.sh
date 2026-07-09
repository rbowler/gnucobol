#!/bin/bash
# Compile and install gnucobol-shift
CURDIR=$(pwd)
if [ "${CURDIR##*/}" != "gnucobol" ]; then
  echo "build should be run from the gnucobol directory"
  exit 1
fi
# Ask for sudo at the beginning to cache credentials for 'make install'
sudo -v
cd build
make
sudo make install
sudo ldconfig
