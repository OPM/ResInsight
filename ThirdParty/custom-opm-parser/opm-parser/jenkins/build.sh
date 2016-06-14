#!/bin/bash

source `dirname $0`/build-opm-parser.sh

ERT_REVISION=master
OPM_COMMON_REVISION=master

build_opm_parser
test $? -eq 0 || exit 1

cp serial/build-opm-parser/testoutput.xml .
