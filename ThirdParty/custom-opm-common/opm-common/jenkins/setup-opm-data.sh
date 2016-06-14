#!/bin/bash

# Predefined by environment
if test -z "$OPM_DATA_ROOT"
then
  OPM_DATA_REVISION="master"
  if grep -q "opm-data=" <<< $ghprbCommentBody
  then
    OPM_DATA_REVISION=pull/`echo $ghprbCommentBody | sed -r 's/.*opm-data=([0-9]+).*/\1/g'`/merge
  fi
  # Not specified in trigger, use shared copy
  if [[ "$OPM_DATA_REVISION" = "master" ]] && [[ ! "$OPM_DATA_ROOT_PREDEFINED" = "" ]]
  then
    if ! test -d $WORKSPACE/deps/opm-data
    then
      cp $OPM_DATA_ROOT_PREDEFINED $WORKSPACE/deps/opm-data -R
    fi
  else
    # Specified in trigger, download it
    source $WORKSPACE/deps/opm-common/jenkins/build-opm-module.sh
    clone_module opm-data $OPM_DATA_REVISION
  fi
else
  if ! test -d $WORKSPACE/deps/opm-data
  then
    cp $OPM_DATA_ROOT $WORKSPACE/deps/opm-data -R
  fi
fi
OPM_DATA_ROOT=$WORKSPACE/deps/opm-data
