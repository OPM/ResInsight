#!/usr/bin/env bash
set -e

pushd . > /dev/null
cd opm-common
mkdir build
cd build
cmake ../
make
popd > /dev/null
