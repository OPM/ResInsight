#!/bin/bash

# $1 = Additional cmake parameters
# $2 = 0 to build and install module, 1 to build and test module
# $3 = Source root of module to build
function build_module {
  cmake $3 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=$2 $1
  test $? -eq 0 || exit 1
  if test $2 -eq 1
  then
    cmake --build .
    test $? -eq 0 || exit 2
    ctest -T Test --no-compress-output
    $WORKSPACE/deps/opm-common/jenkins/convert.py -x $WORKSPACE/deps/opm-common/jenkins/conv.xsl -t . > testoutput.xml
  else
    cmake --build . --target install
  fi
}

# $1 = Name of module
# $2 = git-rev to use for module
function clone_module {
  pushd .
  mkdir -p $WORKSPACE/deps/$1
  cd $WORKSPACE/deps/$1
  git init .
  git remote add origin https://github.com/OPM/$1
  git fetch --depth 1 origin $2:branch_to_build
  git checkout branch_to_build
  test $? -eq 0 || exit 1
  popd
}

# $1 = Module to clone
# $2 = Additional cmake parameters
# $3 = git-rev to use for module
# $4 = Build root
function clone_and_build_module {
  clone_module $1 $3
  pushd .
  mkdir $4/build-$1
  cd $4/build-$1
  test_build=0
  if test -n "$5"
  then
    test_build=$5
  fi
  build_module "$2" $test_build $WORKSPACE/deps/$1
  test $? -eq 0 || exit 1
  popd
}

# Uses pre-filled arrays upstreams, and associativ array upstreamRev
# which holds the revisions to use for upstreams.
function build_upstreams {
  for upstream in ${upstreams[*]}
  do
    echo "Building upstream $upstream=${upstreamRev[$upstream]}"

    # Build upstream and execute installation
    clone_and_build_module $upstream "-DCMAKE_PREFIX_PATH=$WORKSPACE/serial/install -DCMAKE_INSTALL_PREFIX=$WORKSPACE/serial/install" ${upstreamRev[$upstream]} $WORKSPACE/serial
    test $? -eq 0 || exit 1
  done
  test $? -eq 0 || exit 1
}

# $1 - name of the module we are called from
# Uses pre-filled arrays downstreams, and associativ array downstreamRev
# which holds the default revisions to use for downstreams
function build_downstreams {
  pushd .
  cd $WORKSPACE/serial/build-$1
  cmake --build . --target install
  popd

  egrep_cmd="xml_grep --wrap testsuites --cond testsuite $WORKSPACE/serial/build-$1/testoutput.xml"
  for downstream in ${downstreams[*]}
  do
    if grep -q "$downstream=" <<< $ghprbCommentBody
    then
      downstreamRev[$downstream]=pull/`echo $ghprbCommentBody | sed -r "s/.*$downstream=([0-9]+).*/\1/g"`/merge
    fi
    echo "Building downstream $downstream=${downstreamRev[$downstream]}"
    # Build downstream and execute installation
    # Additional cmake parameters:
    # OPM_DATA_ROOT - passed for modules having opm-data based integration tests
    # USE_QUADMATH - used by ewoms to disable quadmath support (makes tests usable)
    clone_and_build_module $downstream "-DCMAKE_PREFIX_PATH=$WORKSPACE/serial/install -DCMAKE_INSTALL_PREFIX=$WORKSPACE/serial/install -DOPM_DATA_ROOT=$OPM_DATA_ROOT -DUSE_QUADMATH=0" ${downstreamRev[$downstream]} $WORKSPACE/serial 1
    code=$?
    # ewoms skips tests in nasty ways. ignore return code
    if [ "$downstream" != "ewoms" ]
    then
      test $code -eq 0 || exit 1
    fi

    # Installation for downstream
    pushd .
    cd $WORKSPACE/serial/build-$downstream
    cmake --build . --target install
    popd
    egrep_cmd="$egrep_cmd $WORKSPACE/serial/build-$downstream/testoutput.xml"
  done

  $egrep_cmd > testoutput.xml
  test $? -eq 0 || exit 1
}
