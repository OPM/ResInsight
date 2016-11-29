# opm-parser jenkins build scripts:

**build.sh**:
This script will build dependencies, then build opm-parser and execute its tests.
It also inspects the $ghbPrBuildComment environmental variable and builds
downstreams if requested. It inspects the $ghbPrBuildComment
environmental variable to obtain a pull request to use for the modules.

It is intended for pre-merge builds of pull requests.

To specify a given pull request to use for upstreams and downstreams,
trigger line needs to contain &lt;module-name&gt;=&lt;pull request number&gt;.

To build with downstreams the trigger line needs to contain 'with downstreams'.
