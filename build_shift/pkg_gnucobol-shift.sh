#!/bin/bash
# Script to build gnucobol-shift package
# Based on files found in gnucobol3_3.2-4_amd64.deb

PKG_NAME=gnucobol-shift

source ./package_functions

# Create directory structure
DIR_NAMES=(
    "bin"
    "share/gnucobol/config"
    "share/gnucobol/copy"
    "share/info"
    "share/man/man1"
    "share/locale/de/LC_MESSAGES"
    "share/locale/en@boldquot/LC_MESSAGES"
    "share/locale/en@quot/LC_MESSAGES"
    "share/locale/es/LC_MESSAGES"
    "share/locale/fr/LC_MESSAGES"
    "share/locale/it/LC_MESSAGES"
    "share/locale/ja/LC_MESSAGES"
    "share/locale/nl/LC_MESSAGES"
    "share/locale/pt/LC_MESSAGES"
    "share/locale/sr/LC_MESSAGES"
    "share/locale/sv/LC_MESSAGES"
    "share/locale/tr/LC_MESSAGES"
)
create_directory_structure "${DIR_NAMES[@]}"

# Copy binary files
echo "Copying binary files..."
BIN_FILES=(
    "cob-config"
    "cobc"
    "cobcrun"
)
copy_files "bin" "${BIN_FILES[@]}"

# Copy config files
echo "Copying config files..."
CONFIG_FILES=(
    "acu-strict.conf"
    "acu.conf"
    "acu.words"
    "alternate.ttbl"
    "bs2000-strict.conf"
    "bs2000.conf"
    "bs2000.words"
    "cobol2002.conf"
    "cobol2002.words"
    "cobol2014.conf"
    "cobol2014.words"
    "cobol85.conf"
    "cobol85.words"
    "default.conf"
    "default.ttbl"
    "ebcdic500_ascii7bit.ttbl"
    "ebcdic500_ascii8bit.ttbl"
    "ebcdic500_latin1.ttbl"
    "gcos-strict.conf"
    "gcos.conf"
    "gcos.words"
    "ibm-strict.conf"
    "ibm.conf"
    "ibm.words"
    "lax.conf-inc"
    "mf-strict.conf"
    "mf.conf"
    "mf.words"
    "mvs-strict.conf"
    "mvs.conf"
    "mvs.words"
    "realia-strict.conf"
    "realia.conf"
    "realia.words"
    "rm-strict.conf"
    "rm.conf"
    "rm.words"
    "runtime.cfg"
    "runtime_empty.cfg"
    "xopen.conf"
)
copy_files "share/gnucobol/config" "${CONFIG_FILES[@]}"

# Copy copy files
echo "Copying copy files..."
COPY_FILES=(
    "gcwindow.cpy"
    "screenio.cpy"
    "sqlca.cpy"
    "sqlda.cpy"
    "xfhfcd.cpy"
    "xfhfcd3.cpy"
)
copy_files "share/gnucobol/copy" "${COPY_FILES[@]}"

# Copy info file
echo "Copying info file..."
INFO_FILES=(
    "gnucobol.info"
)
copy_files "share/info" "${INFO_FILES[@]}"

# Copy man pages
echo "Copying man pages..."
MAN_FILES=(
    "cob-config.1"
    "cobc.1"
    "cobcrun.1"
)
copy_files "share/man/man1" "${MAN_FILES[@]}"

# Copy locale files
echo "Copying locale files..."
LOCALES=(
    "de/LC_MESSAGES/gnucobol.mo"
    "en@boldquot/LC_MESSAGES/gnucobol.mo"
    "en@quot/LC_MESSAGES/gnucobol.mo"
    "es/LC_MESSAGES/gnucobol.mo"
    "fr/LC_MESSAGES/gnucobol.mo"
    "it/LC_MESSAGES/gnucobol.mo"
    "ja/LC_MESSAGES/gnucobol.mo"
    "nl/LC_MESSAGES/gnucobol.mo"
    "pt/LC_MESSAGES/gnucobol.mo"
    "sr/LC_MESSAGES/gnucobol.mo"
    "sv/LC_MESSAGES/gnucobol.mo"
    "tr/LC_MESSAGES/gnucobol.mo"
)
copy_files "share/locale" "${LOCALES[@]}"

echo "File copy operation completed!"

build_package "../build"
