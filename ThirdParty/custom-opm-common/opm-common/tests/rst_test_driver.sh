#!/bin/bash
set -e

rst_deck=$1
opmhash=$2
deck=$3

pushd $(mktemp -d)

${rst_deck} -m copy -s ${deck} HISTORY:0 output/CASE_COPY.DATA
${opmhash} -S ${deck} output/CASE_COPY.DATA

${rst_deck} -m share -s ${deck} HISTORY:0 > CASE_STDOUT.DATA
${opmhash} -S ${deck} CASE_STDOUT.DATA

pushd output
chmod -R a-w *
popd

${rst_deck} -m copy -s output/CASE_COPY.DATA HISTORY:0 output/CASE_SHARE.DATA
${opmhash} -S ${deck} output/CASE_SHARE.DATA
popd
