# opm-parser jenkins build scripts:

**build-opm-parser.sh**:
This is a helper script which contains a function for building,
testing and cloning opm-parser and its dependencies.

**build.sh**:
This script will build dependencies, then build opm-parser and execute its tests.
It is intended for post-merge builds of the master branch.

**build-pr.sh**:
This script will build dependencies, then build opm-parser and execute its tests.
It inspects the $ghbPrBuildComment environmental variable to obtain a pull request
to use for for ert and opm-common (defaults to master)
and then builds $sha1 of opm-parser.

It is intended for pre-merge builds of pull requests.

You specify a given pull request to use for opm-common through the trigger.
The trigger line needs to contain ert=&lt;pull request number&gt; and/or
opm-common=&lt;pull request number&gt;.
