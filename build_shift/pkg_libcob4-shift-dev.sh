#!/bin/bash
# Script to build libcob4-shift-dev package
# Based on files found in libcob4-dev_3.2-4_amd64.deb

PKG_NAME=libcob4-shift-dev

source ./package_functions

# Create directory structure
DIR_NAMES=(
    "include/libcob"
    "lib"
)
create_directory_structure "${DIR_NAMES[@]}"

# Copy header files
echo "Copying header files..."
HEADER_FILES=(
    "libcob/cobgetopt.h"
    "libcob/common.h"
    "libcob/exception-io.def"
    "libcob/exception.def"
    "libcob/statement.def"
    "libcob/version.h"
    "libcob.h"
)
copy_files "include" "${HEADER_FILES[@]}"

# Copy library archive
echo "Copying library archive..."
LIBRARY_FILES=(
    "libcob.a"
)
copy_files "lib" "${LIBRARY_FILES[@]}"

# Create symlink for libcob.so -> libcob.so.4.2.0
echo "Creating symlinks..."
create_symlink lib libcob.so libcob.so.4.2.0

echo "File copy operation completed!"

build_package "../build"
