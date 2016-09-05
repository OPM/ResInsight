#!/bin/sh

set -e

mkdir -p opm-flowdiagnostics/build

(cd opm-flowdiagnostics/build
 cmake ../

 make)
