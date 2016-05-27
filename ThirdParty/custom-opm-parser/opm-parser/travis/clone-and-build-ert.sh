#!/usr/bin/env bash
set -e

pushd . > /dev/null
git clone https://github.com/Ensembles/ert.git
cd ert
mkdir build
cd build
cmake ../devel/
make
popd > /dev/null
