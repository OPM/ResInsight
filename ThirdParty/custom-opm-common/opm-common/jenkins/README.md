# opm-common jenkins build scripts:

**build-opm-module.sh**:
This is a helper script which contains functions for building,
testing and cloning modules.

**build.sh**:
This expects to run on a jenkins instance with opm-common as the 'origin' remote.

It will build and test opm-common. It can be used both for post-merge builds
of the master branch and for a github pull request builder job.
