#!/bin/bash

function build_opm_parser {
  # Build ERT
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
  make install
  popd

  # Build opm-common
  pushd .
  mkdir -p $WORKSPACE/deps/opm-common
  cd $WORKSPACE/deps/opm-common
  git init .
  git remote add origin https://github.com/OPM/opm-common
  git fetch --depth 1 origin $OPM_COMMON_REVISION:branch_to_build
  test $? -eq 0 || exit 1
  git checkout branch_to_build
  popd
  source $WORKSPACE/deps/opm-common/jenkins/build-opm-module.sh

  pushd .
  mkdir serial/build-opm-common
  cd serial/build-opm-common
  build_module "-DCMAKE_INSTALL_PREFIX=$WORKSPACE/serial/install" 0 $WORKSPACE/deps/opm-common
  popd

  # Setup opm-data
  source $WORKSPACE/deps/opm-common/jenkins/setup-opm-data.sh

  # Build opm-parser
  pushd .
  mkdir serial/build-opm-parser
  cd serial/build-opm-parser
  build_module "-DCMAKE_PREFIX_PATH=$WORKSPACE/serial/install -DCMAKE_INSTALL_PREFIX=$WORKSPACE/serial/install -DOPM_DATA_ROOT=$OPM_DATA_ROOT" 1 $WORKSPACE
  test $? -eq 0 || exit 1
  popd
}
