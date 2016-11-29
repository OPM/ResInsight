#!/bin/sh
set -e

sh ./opm-flowdiagnostics/travis/build-opm-flowdiagnostics.sh

(cd opm-flowdiagnostics/build
 ctest --output-on-failure)
