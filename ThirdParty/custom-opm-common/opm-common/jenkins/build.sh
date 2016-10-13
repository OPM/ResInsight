#!/bin/bash

source `dirname $0`/build-opm-module.sh

# Create symlink so build_module can find the test result converter
mkdir deps
ln -sf $WORKSPACE deps/opm-common

# Downstreams and revisions
declare -a downstreams
downstreams=(ert
             opm-parser
             opm-output
             opm-material
             opm-core
             opm-grid
             opm-simulators
             opm-upscaling
             ewoms)

declare -A downstreamRev
downstreamRev[ert]=master
downstreamRev[opm-parser]=master
downstreamRev[opm-material]=master
downstreamRev[opm-core]=master
downstreamRev[opm-grid]=master
downstreamRev[opm-output]=master
downstreamRev[opm-simulators]=master
downstreamRev[opm-upscaling]=master
downstreamRev[ewoms]=master

parseRevisions
printHeader opm-common

# Setup opm-data if necessary
if grep -q "with downstreams" <<< $ghprbCommentBody
then
  source $WORKSPACE/deps/opm-common/jenkins/setup-opm-data.sh
fi

build_module_full opm-common
