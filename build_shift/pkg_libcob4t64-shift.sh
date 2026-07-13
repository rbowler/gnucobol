#!/bin/bash
# Script to build libcob4t64-shift package
# Based on files found in libcob4t64_3.2-3_amd64.deb

PKG_NAME=libcob4t64-shift

source ./package_functions

# Create directory structure
DIR_NAMES=(
    "lib/gnucobol"
)
create_directory_structure "${DIR_NAMES[@]}"

# Copy library files
echo "Copying library files..."
LIBRARY_FILES=(
    "libcob.so.4.2.0"
)
copy_files "lib" "${LIBRARY_FILES[@]}"

LIBRARY_DIRS=(
    "gnucobol"
)
copy_directories "lib" "${LIBRARY_DIRS[@]}"

# Create symlink for libcob.so.4 -> libcob.so.4.2.0
echo "Creating symlinks..."
create_symlink lib libcob.so.4 libcob.so.4.2.0

build_package "../build"
