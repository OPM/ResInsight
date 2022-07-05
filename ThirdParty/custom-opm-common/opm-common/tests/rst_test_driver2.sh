#!/bin/bash
set -e

rst_deck=$1
opmi=$2
unif_deck=$3
single_deck=$4
unified_rst_file=$5
single_rst_file=$6

pushd $(mktemp -d)

# Happy path using unified restart file from source location
${rst_deck} -s ${unif_deck} ${unified_rst_file}:10 output/RESTART_UNIF.DATA
${opmi} output/RESTART_UNIF.DATA

# Happy path using single restart file with absolute path
${rst_deck} -s ${single_deck} ${single_rst_file} output/RESTART_SINGLE_ABS.DATA
${opmi} output/RESTART_SINGLE_ABS.DATA

mkdir rst
cp ${single_rst_file} rst/.
single_rst_file="$(basename -- ${single_rst_file})"

# Happy path using single restart file wth relative path from rst/
${rst_deck} -s ${single_deck} rst/${single_rst_file} output/RESTART_SINGLE_RELATIVE.DATA
${opmi} output/RESTART_SINGLE_RELATIVE.DATA


assert_error() {
    retVal=$?
    if [ $retVal -eq 0 ]; then
        echo "Error"
        exit 1
    fi
}


set +e
# Test two error conditions

# Deck with UNIFIN and pass a single .X0010 restart file
${rst_deck} -s ${unif_deck} rst/${single_rst_file} output/RESTART_SINGLE_RELATIVE.DATA
assert_error

# Ask for wrong restart number from unified restart file
${rst_deck} -s ${unif_deck} ${unified_rst_file}:20 output/RESTART_ERROR.DATA
${opmi} output/RESTART_ERROR.DATA
assert_error

popd
