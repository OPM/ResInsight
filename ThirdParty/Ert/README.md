# libecl [![Build Status](https://travis-ci.org/Statoil/libecl.svg?branch=master)](https://travis-ci.org/Statoil/libecl)


*libecl* is a package for reading and writing the result files from
the Eclipse reservoir simulator. The file types covered are the
restart, init, rft, summary and grid files. Both unified and
non-unified and formatted and unformatted files are supported.

*libecl* is mainly developed on *Linux* and *OS X*, in addition there
is a portability layer which ensures that most of the functionality is
available on *Windows*. The main functionality is written in C/C++, and
should typically be linked in in other compiled programs. *libecl* was
initially developed as part of the [Ensemble Reservoir
Tool](http://github.com/Statoil/ert), other applications using
*libecl* are the reservoir simulator flow and Resinsight from the [OPM
project](http://github.com/OPM/).

In addition to the compiled C/C++ code there are Python wrappers which make most
of the *libecl* functionality available from Python. For small interactive
scripts, forward models e.t.c. this is the recommended way to use *libecl*
functionality. You decide wether to build include the Python wrappers when
configuring the `cmake` build - pass the option `-DENABLE_PYTHON=ON` to enable
Python wrappers, by default the Python wrappers are *not* included.

By default `cmake` is configured to install to a system location like
`/usr/local`, if you want to install to an alternative location that should be
configured with `-DCMAKE_INSTALL_PREFIX=/path/to/install`.

### Alternative 1: Building without Python ###
*libecl* uses CMake as build system:

```bash
git clone https://github.com/Statoil/libecl
cd libecl
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
make
make install
```
If you intend to develop and change *libecl* you should build the tests
by passing `-DBUILD_TESTS=ON` and run the tests with `ctest`.


### Alternative 2: Building with Python ###
To build *libecl* with Python wrappers you just pass the option
`-DENABLE_PYTHON=ON` when running `cmake`. In addition you need to install some
dependencies first. Python is not a compiled language, but the `cmake` configuration contains a
rudimentary "build system" which does a basic Python syntax check and configures some
files to correctly set up the interaction between the Python classes and the
shared libraries built from the C code:
```bash
git clone https://github.com/Statoil/libecl
cd libecl
sudo pip install -r requirements.txt
mkdir build
cd build
cmake .. -DENABLE_PYTHON=ON -DCMAKE_INSTALL_PREFIX=/path/to/install 
make
make install
```

After you have installed the Python wrappers you must tell `Python` where to
find the package[1]:

```bash
export PYTHONPATH=/path/to/install/lib/python2.7/site-packages:$PYTHONPATH
export LD_LIBRARY_PATH=/path/to/install/lib64:$LD_LIBRARY_PATH
```

Then you can fire up your Python interpreter and try it out:
```
from ecl.summary import EclSum
import sys

summary = EclSum(sys.argv[1])
fopt = summary.numpy_vector("FOPT")

```


[1]: The exact paths here will depend on your system and Python version. The example given is for a RedHat system with Python version 2.7.
