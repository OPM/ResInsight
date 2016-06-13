#!/bin/bash
set -e

# This script should build all the OPM dependencies which are installed from source.


# The build_dune function should take the module name as the first
# argument. By default the script will clone the source from:
#
#    https://github.com/dune-project/${project}.git
#
# But you can optionally supply a git url as second argument, i.e.
#
#    build_dune  dune-alugrid https://gitlab.dune-project.org/extensions/dune-alugrid.git
#
# to build the dune-alugrid module which is not found at github.

function build_dune {
   project=$1
   if [[ $# -eq 1 ]]; then
       url=https://github.com/dune-project/${project}.git
   else
       url=$2
   fi
   pushd . > /dev/null
   git clone ${url}
   cd ${project}
   git checkout tags/v2.3.1
   mkdir build
   cd build
   cmake ../
   make
   popd > /dev/null
}



function build_superlu {
   pushd . > /dev/null
   git clone https://github.com/starseeker/SuperLU.git
   cd SuperLU
   mkdir build
   cd build
   cmake -D CMAKE_INSTALL_PREFIX=.. -D SUPERLU_BUILD_EXAMPLES=OFF -D SUPERLU_ENABLE_TESTING=OFF ../
   make install
   popd > /dev/null
}



function build_ert {
    git clone https://github.com/Ensembles/ert.git
    mkdir -p ert/build
    pushd ert/build > /dev/null
    cmake ../devel && make
    popd > /dev/null
}


#################################################################

build_superlu
build_ert

build_dune dune-common
build_dune dune-istl
build_dune dune-geometry
build_dune dune-grid



