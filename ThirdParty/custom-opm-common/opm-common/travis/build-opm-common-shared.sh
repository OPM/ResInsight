#!/usr/bin/env bash
set -e

pushd . > /dev/null
cd opm-common
mkdir build
cd build
cmake ../ -DBUILD_SHARED_LIBS=ON
make
popd > /dev/null
