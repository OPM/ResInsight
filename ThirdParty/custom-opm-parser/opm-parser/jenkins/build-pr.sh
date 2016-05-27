#!/bin/bash

source `dirname $0`/build-opm-parser.sh

# Upstream revisions
ERT_REVISION=master
OPM_COMMON_REVISION=master

if grep -q "ert=" <<< $ghprbCommentBody
then
  ERT_REVISION=pull/`echo $ghprbCommentBody | sed -r 's/.*ert=([0-9]+).*/\1/g'`/merge
fi
if grep -q "opm-common=" <<< $ghprbCommentBody
then
  OPM_COMMON_REVISION=pull/`echo $ghprbCommentBody | sed -r 's/.*opm-common=([0-9]+).*/\1/g'`/merge
fi

echo "Building with ert=$ERT_REVISION opm-common=$OPM_COMMON_REVISION opm-parser=$sha1"

build_opm_parser
test $? -eq 0 || exit 1

# If no downstream builds we are done
if ! grep -q "with downstreams" <<< $ghprbCommentBody
then
  cp serial/build-opm-parser/testoutput.xml .
  exit 0
fi

source $WORKSPACE/deps/opm-common/jenkins/build-opm-module.sh

# Downstream revisions
declare -a downstreams
downstreams=(opm-material
             opm-core
             opm-grid
             opm-output
             opm-simulators
             opm-upscaling
             ewoms)

declare -A downstreamRev
downstreamRev[opm-material]=master
downstreamRev[opm-core]=master
downstreamRev[opm-grid]=master
downstreamRev[opm-output]=master
downstreamRev[opm-simulators]=master
downstreamRev[opm-upscaling]=master
downstreamRev[ewoms]=master

build_downstreams opm-parser

test $? -eq 0 || exit 1
