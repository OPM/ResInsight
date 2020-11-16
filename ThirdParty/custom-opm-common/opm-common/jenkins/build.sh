#!/bin/bash

source `dirname $0`/build-opm-module.sh

# Create symlink so build_module can find the test result converter
mkdir deps
ln -sf $WORKSPACE deps/opm-common

# No upstreams
declare -a upstreams
declare -A upstreamRev

# Downstreams and revisions
declare -a downstreams
downstreams=(opm-material
             opm-grid
             opm-models
             opm-simulators
             opm-upscaling
             )

declare -A downstreamRev
downstreamRev[opm-material]=master
downstreamRev[opm-grid]=master
downstreamRev[opm-models]=master
downstreamRev[opm-simulators]=master
downstreamRev[opm-upscaling]=master

parseRevisions
printHeader opm-common

# Setup opm-data
source $WORKSPACE/deps/opm-common/jenkins/setup-opm-tests.sh

build_module_full opm-common
