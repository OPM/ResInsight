#!/usr/bin/env bash
set -e

pushd . > /dev/null
opm-common/travis/build-opm-common.sh
cd opm-common/build
ctest --output-on-failure
popd > /dev/null
