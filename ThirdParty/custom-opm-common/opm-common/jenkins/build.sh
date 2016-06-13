#!/bin/bash

source `dirname $0`/build-opm-module.sh

# Create symlink so build_module can find the test result converter
mkdir deps
ln -sf $WORKSPACE deps/opm-common

pushd .
mkdir -p serial/build-opm-common
cd serial/build-opm-common
build_module "-DCMAKE_INSTALL_PREFIX=$WORKSPACE/serial/install" 1 $WORKSPACE
test $? -eq 0 || exit 1
popd

# If no downstream builds we are done
if ! grep -q "with downstreams" <<< $ghprbCommentBody
then
  cp serial/build-opm-common/testoutput.xml .
  exit 0
fi

ERT_REVISION=master

if grep -q "ert=" <<< $ghprbCommentBody
then
  ERT_REVISION=pull/`echo $ghprbCommentBody | sed -r 's/.*ert=([0-9]+).*/\1/g'`/merge
fi

source $WORKSPACE/deps/opm-common/jenkins/setup-opm-data.sh

# Downstream revisions
declare -a downstreams
downstreams=(opm-parser
             opm-output
             opm-material
             opm-core
             opm-grid
             opm-simulators
             opm-upscaling
             ewoms)

declare -A downstreamRev
downstreamRev[opm-parser]=master
downstreamRev[opm-material]=master
downstreamRev[opm-core]=master
downstreamRev[opm-grid]=master
downstreamRev[opm-output]=master
downstreamRev[opm-simulators]=master
downstreamRev[opm-upscaling]=master
downstreamRev[ewoms]=master

# Build ERT
echo "Building downstream ert=$ERT_REVISION"
pushd .
mkdir -p $WORKSPACE/deps/ert
cd $WORKSPACE/deps/ert
git init .
git remote add origin https://github.com/Ensembles/ert
git fetch --depth 1 origin $ERT:branch_to_build
test $? -eq 0 || exit 1
git checkout branch_to_build
popd

pushd .
mkdir -p serial/build-ert
cd serial/build-ert
cmake $WORKSPACE/deps/ert/devel -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$WORKSPACE/serial/install
cmake --build . --target install
popd

build_downstreams opm-common

test $? -eq 0 || exit 1
