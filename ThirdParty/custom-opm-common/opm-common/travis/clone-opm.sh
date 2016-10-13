#!/usr/bin/env bash
set -e

project_list=(opm-data opm-parser opm-material ewoms opm-core opm-output opm-grid opm-simulators opm-upscaling)

# Will clone all the projects *except* the one project given as
# commandline argument; that has typically been checked out by travis
# already. Will not clone opm-commone because that should already be
# present, either because it is the current repository - or because
# that must be cloned specifically from the other modules first.


function clone_project {
    url=https://github.com/OPM/${1}.git
    git clone $url
}


for project in "${project_list[@]}"; do
    if [ "$project" != $1 ]; then
        clone_project $project
    fi
done



