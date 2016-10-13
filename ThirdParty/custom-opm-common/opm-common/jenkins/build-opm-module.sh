#!/bin/bash

declare -A configurations

# Parse revisions from trigger comment and setup arrays
# Depends on: 'upstreams', upstreamRev',
#             'downstreams', 'downstreamRev',
#             'ghprbCommentBody',
#             'CONFIGURATIONS', 'TOOLCHAINS'
function parseRevisions {
  for upstream in ${upstreams[*]}
  do
    if grep -qi "$upstream=" <<< $ghprbCommentBody
    then
      upstreamRev[$upstream]=pull/`echo $ghprbCommentBody | sed -r "s/.*${upstream,,}=([0-9]+).*/\1/g"`/merge
    fi
  done
  if grep -q "with downstreams" <<< $ghprbCommentBody
  then
    for downstream in ${downstreams[*]}
    do
      if grep -qi "$downstream=" <<< $ghprbCommentBody
      then
        downstreamRev[$downstream]=pull/`echo $ghprbCommentBody | sed -r "s/.*${downstream,,}=([0-9]+).*/\1/g"`/merge
      fi
    done
  fi

  # Default to a serial build if no types are given
  if test -z "$BTYPES"
  then
    BTYPES="serial"
  fi

  # Convert to arrays for easy looping
  declare -a BTYPES_ARRAY
  for btype in $BTYPES
  do
    BTYPES_ARRAY=($BTYPES_ARRAY $btype)
  done
  TOOLCHAIN_ARRAY=($CMAKE_TOOLCHAIN_FILES)
  for index in ${!BTYPES_ARRAY[*]}
  do
    key=${BTYPES_ARRAY[$index]}
    data=${TOOLCHAIN_ARRAY[$index]}
    configurations[$key]=$data
  done
}

# Print revisions and configurations
# $1 = Name of main module
# Depends on: 'upstreams', upstreamRev',
#             'downstreams', 'downstreamRev',
#             'ghprbCommentBody',
#             'configurations', 'sha1'
function printHeader {
  echo -e "Repository revisions:"
  for upstream in ${upstreams[*]}
  do
    echo -e "\t   [upstream] $upstream=${upstreamRev[$upstream]}"
  done
  echo -e "\t[main module] $1=$sha1"
  if grep -q "with downstreams" <<< $ghprbCommentBody
  then
    for downstream in ${downstreams[*]}
    do
      echo -e "\t [downstream] $downstream=${downstreamRev[$downstream]}"
    done
  fi

  echo "Configurations to process:"
  for conf in ${!configurations[@]}
  do
    echo -e "\t$conf=${configurations[$conf]}"
  done
}

# $1 = Additional cmake parameters
# $2 = 0 to build and install module, 1 to build and test module
# $3 = Source root of module to build
function build_module {
  cmake $3 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=$2 -DCMAKE_TOOLCHAIN_FILE=${configurations[$configuration]} $1
  test $? -eq 0 || exit 1
  if test $2 -eq 1
  then
    cmake --build .
    test $? -eq 0 || exit 2
    ctest -T Test --no-compress-output

    # Convert to junit format
    $WORKSPACE/deps/opm-common/jenkins/convert.py -x $WORKSPACE/deps/opm-common/jenkins/conv.xsl -t . > testoutput.xml

    if ! grep -q "with downstreams" <<< $ghprbCommentBody
    then
      # Add configuration name
      sed -e "s/classname=\"TestSuite\"/classname=\"${configuration}\"/g" testoutput.xml > $WORKSPACE/$configuration/testoutput.xml
    fi
  else
    cmake --build . --target install
  fi
}

# $1 = Name of module
# $2 = git-rev to use for module
function clone_module {
  # Already cloned by an earlier configuration
  test -d $WORKSPACE/deps/$1 && return 0
  pushd .
  mkdir -p $WORKSPACE/deps/$1
  cd $WORKSPACE/deps/$1
  git init .
  if [ "$1" == "ert" ]
  then
    git remote add origin https://github.com/Ensembles/$1
  else
    git remote add origin https://github.com/OPM/$1
  fi
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
  mkdir -p $4/build-$1
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
    echo "Building upstream $upstream=${upstreamRev[$upstream]} configuration=$configuration"
    # Build upstream and execute installation
    clone_and_build_module $upstream "-DCMAKE_PREFIX_PATH=$WORKSPACE/$configuration/install -DCMAKE_INSTALL_PREFIX=$WORKSPACE/$configuration/install" ${upstreamRev[$upstream]} $WORKSPACE/$configuration
    test $? -eq 0 || exit 1
  done
  test $? -eq 0 || exit 1
}

# $1 - name of the module we are called from
# Uses pre-filled arrays downstreams, and associativ array downstreamRev
# which holds the default revisions to use for downstreams
function build_downstreams {
  pushd .
  cd $WORKSPACE/$configuration/build-$1
  cmake --build . --target install
  popd

  egrep_cmd="xml_grep --wrap testsuites --cond testsuite $WORKSPACE/$configuration/build-$1/testoutput.xml"
  for downstream in ${downstreams[*]}
  do
    echo "Building downstream $downstream=${downstreamRev[$downstream]} configuration=$configuration"
    # Build downstream and execute installation
    clone_and_build_module $downstream "-DCMAKE_PREFIX_PATH=$WORKSPACE/$configuration/install -DCMAKE_INSTALL_PREFIX=$WORKSPACE/$configuration/install -DOPM_DATA_ROOT=$OPM_DATA_ROOT" ${downstreamRev[$downstream]} $WORKSPACE/$configuration 1
    test $? -eq 0 || exit 1

    # Installation for downstream
    pushd .
    cd $WORKSPACE/$configuration/build-$downstream
    cmake --build . --target install
    popd
    egrep_cmd="$egrep_cmd $WORKSPACE/$configuration/build-$downstream/testoutput.xml"
  done

  $egrep_cmd > $WORKSPACE/$configuration/testoutput.xml

  # Add testsuite name
  sed -e "s/classname=\"TestSuite\"/classname=\"${configuration}\"/g" -i $WORKSPACE/$configuration/testoutput.xml

  test $? -eq 0 || exit 1
}

# $1 = Name of main module
function build_module_full {
  for configuration in ${!configurations[@]}
  do
    # Build upstream modules
    build_upstreams

    # Build main module
    pushd .
    mkdir -p $configuration/build-$1
    cd $configuration/build-$1
    echo "Building main module $1=$sha1 configuration=$configuration"
    build_module "-DCMAKE_INSTALL_PREFIX=$WORKSPACE/$configuration/install -DOPM_DATA_ROOT=$OPM_DATA_ROOT" 1 $WORKSPACE
    test $? -eq 0 || exit 1
    popd

    # If no downstream builds we are done
    if grep -q "with downstreams" <<< $ghprbCommentBody
    then
      build_downstreams $1
      test $? -eq 0 || exit 1
    fi
  done
}
