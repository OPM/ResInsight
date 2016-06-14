#!/bin/bash
set -e

build_order=(opm-common opm-parser opm-material opm-output opm-core opm-grid opm-simulators opm-upscaling)

# This shell script should be started with the name of a module as
# only only command line argument. It will start by building all
# upstream modules, then it will build and test the module of interest
# and all downstream modules.
#
# Before invoking this script all the modules should have been cloned
# in sibling directories, so that this script will see this directory
# structure:
#
#   opm-common/
#   opm-parser/
#   opm-material/
#   opm-output/
#   opm-core/
#   opm-grid/
#   opm-simulators/
#   opm-upscaling/
#
#
# This can typically be achived by using the 'clone-opm.sh' script.



function upstream_build {
    project=${1}
    echo "Building: ${project}"
    mkdir -p ${project}/build
    pushd ${project}/build > /dev/null
    cmake ../ -DENABLE_PYTHON=ON -DBUILD_TESTING=OFF -DSILENCE_EXTERNAL_WARNINGS=True
    make 
    popd > /dev/null
}


function downstream_build_and_test {
    project=${1}
    echo "Building and testing: ${project}"
    mkdir -p ${project}/build
    pushd ${project}/build > /dev/null
    # The build commands cmake, make and ctest must be given as
    # separate commands and not chained with &&. If chaining with &&
    # is used the 'set -e' does not exit on first error.
    cmake ../ -DENABLE_PYTHON=ON -DBUILD_TESTING=ON -DSILENCE_EXTERNAL_WARNINGS=True
    make
    ctest --output-on-failure
    popd > /dev/null
}


for i in "${!build_order[@]}"; do
   if [[ "${build_order[$i]}" = "$1" ]]; then
       project_index=$i
   fi
done

if [[ -z ${project_index} ]]; then
   echo "${0}: Project: ${1} not recognized."
   exit 1
fi



build_index=0


while [ $build_index -lt ${project_index} ];
do
    project=${build_order[$build_index]}
    upstream_build ${project}
    build_index=$((build_index + 1)) 
done



while [ $build_index -lt ${#build_order[@]} ]
do
    project=${build_order[$build_index]}
    downstream_build_and_test ${project}
    build_index=$((build_index + 1)) 
done
      
