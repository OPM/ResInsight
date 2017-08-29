# libecl [![Build Status](https://travis-ci.org/Statoil/libecl.svg?branch=master)](https://travis-ci.org/Statoil/libecl)


*libecl* is a package for reading and writing the result files from
the Eclipse reservoir simulator. The file types covered are the
restart, init, rft, summary and grid files. Both unified and
non-unified and formatted and unformatted files are supported.

*libecl* is mainly developed on *Linux* and *OS X*, in addition there
is a portability layer which ensures that most of the functionality is
available on *Windows*. The main functionality is written in C, and
should typically be linked in in other compiled programs. *libecl* was
initially developed as part of the [Ensemble Reservoir
Tool](http://github.com/Statoil/ert), other applications using
*libecl* are the reservoir simulator flow and Resinsight from the [OPM
project](http://github.com/OPM/).

In addition to the C code there are Python wrappers which make most of
the *libecl* functionality available from Python. For small interactive
scripts, forward models e.t.c. this is recommended way to use *libecl*
functionality.


### Compiling the C code ###
*libecl* uses CMake as build system:

```bash
git clone https://github.com/Statoil/*libecl*.git
cd *libecl*
mkdir build
cd build
cmake ..
make
```
If you intend to develop and change *libecl* you should build the tests
by passing `-DBUILD_TESTS=ON` and run the tests with `ctest`.



### Compiling the Python code ###

Python is not a compiled language, but there is a basic "build system"
which does a basic Python syntax check and configures some files to
correctly set up the interaction between the Python classes and the
shared libraries built from the C code.

You need to install some Python requirements before the Python code
will work:

    sudo pip install -r requirements.txt

The Python + cmake interaction is handled in a separate project called
[pycmake](https://github.com/Statoil/pycmake); you can either install
that manually or use the git submodule functionality to fetch the
correct version of `pycmake` into your *libecl* code:

    git submodule update --init pycmake



