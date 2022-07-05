#!/bin/bash

# Predefined by environment
if test -z "$OPM_TESTS_ROOT"
then
  OPM_TESTS_REVISION="master"
  if grep -q "opm-tests=" <<< $ghprbCommentBody
  then
    OPM_TESTS_REVISION=pull/`echo $ghprbCommentBody | sed -r 's/.*opm-tests=([0-9]+).*/\1/g'`/merge
  fi
  # Not specified in trigger, use shared copy
  if [[ "$OPM_TESTS_REVISION" = "master" ]] && [[ ! "$OPM_TESTS_ROOT_PREDEFINED" = "" ]]
  then
    if ! test -d $WORKSPACE/deps/opm-tests
    then
      cp $OPM_TESTS_ROOT_PREDEFINED $WORKSPACE/deps/opm-tests -R
      pushd $WORKSPACE/deps/opm-tests
      echo "opm-tests revision: `git rev-parse HEAD`"
      popd
    fi
  else
    # We need a full repo checkout
    cp $OPM_TESTS_ROOT_PREDEFINED $WORKSPACE/deps/opm-tests -R
    pushd $WORKSPACE/deps/opm-tests
    # Then we fetch the PR branch
    git remote add PR https://github.com/OPM/opm-tests
    git fetch --depth 1 PR $OPM_TESTS_REVISION:branch_to_build
    git checkout branch_to_build
    popd
  fi
else
  if ! test -d $WORKSPACE/deps/opm-tests
  then
    cp $OPM_TESTS_ROOT $WORKSPACE/deps/opm-tests -R
    pushd $WORKSPACE/deps/opm-tests
    echo "opm-tests-revision: `git rev-parse HEAD`"
    popd
  fi
fi
OPM_TESTS_ROOT=$WORKSPACE/deps/opm-tests
