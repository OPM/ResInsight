#!/bin/sh


target=$1

if [ -z "target" ]
then
    target=.
    echo "Will remove lock files from current directory."
fi

find $target -name "*.lock_*"  -type d -exec rm -rf {} +