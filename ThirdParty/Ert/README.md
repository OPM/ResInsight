# libecl [![Build Status](https://travis-ci.org/Statoil/libecl.svg?branch=master)](https://travis-ci.org/Statoil/libecl)


## Coming from OPM?

`libecl` is a package for handling an ensemble of reservoir models, an
important part of that is beeing able to read and write the files from
standard reservoir applications; `libecl` has quite extensive support for
reading and writing the result files from the ECLIPSE reservoir
simulator. The capabilities to read and write ECLIPSE result files is
used by the OPM simulator codes.

The `libecl` build system has many configuration options, but when
compiling for OPM you should be able to use the all defaults route:

```bash
git clone https://github.com/Statoil/libecl.git
cd libecl
mkdir build
cd build
cmake ..
make
```

The OPM build system can find an `libecl` distribution in the sibling
location, i.e. if you have cloned ert beside the opm modules like:

```
libecl/
opm-common/
opm-parser/
opm-material/
...
```

The opm build system will find the ert distribution in-place,
otherwise you should install ert with 'make install' and the normal
cmake machinery of the opm build system should find it.

```
------------------------------------------------------------------------

                    _________________________________
                   /                                 \
                   |   ______   ______   _______     |
                   |  |  ____| |  __  \ |__   __|    |
                   |  | |__    | |__) |    | |       |
                   |  |  __|   |  _  /     | |       |
                   |  | |____  | | \ \     | |       |
                   |  |______| |_|  \_\    |_|       |
                   |                                 |
                   |  Ensemble based Reservoir Tool  |
                   \_________________________________/


------------------------------------------------------------------------
```

1. `libecl`
2. ECLIPSE utilities.
3. Building `libecl`
    1. CMake settings you might want to adjust
4. The code:
    1. The different libraries
    2. The general structure
    3. Python wrappers
5. Tests

------------------------------------------------------------------------

## 1. `libecl`

`libecl` - Ensemble based Reservoir Tool is a tool for managing an ensemble
of reservoir models. The initial motivation for creating `libecl` was a as
tool to do assisted history matching with the Ensemble Kalman Filter
(EnKF). Very briefly, the process of using EnKF for history matching
can be summarized as:

1. Sample initial reservoir parameters from a (Gaussian) initial
distribution.
2. Simulate the ensemble of of reservoir forward in time through a
part of the historical period for which data is available.
3. Load the results, compare with the observed data, update the
parameters and the state of reservoir by filtering out the most
inacurate results, and restart the forward simulations.

This recipe is quite complex technically, and in particular involves
the ability to read and write input and output files from the
reservoir simulator (i.e. ECLIPSE in the case of `libecl`), run simulations
with arbitrary external programs, plotting data and so on. This
implies that a quite significant technical machinery must be in place
before the EnKF algorithm as such can be utilizied. This in particular
applies to real industry reservoir models, where typically
imperfections of all kinds flourish.

Despite the fact that the initial motivation for creating `libecl` was to
be able to use the EnKF algorithm for history matching, `libecl` is
currently more used with the Ensemble Smoother and also purely as a
workflow manager, i.e. herding a large collection of reservoir models
through the required simulations steps.


## 2. ECLIPSE Utilities

`libecl` has a quite large amount of code devoted to reading and writing
the ECLIPSE output files (grid/rft/restart/init/summary). In addition,
there is also reasonable support for reading and writing the grdecl
input files, but there is no general .DATA file parser. The ability to
read and write ECLIPSE output files is valuable in many reservoir
applications, and it is possible to only build and use the libecl
(with support libraries) library for working with ECLIPSE files. In
fact, the default build setup is to only build the ECLIPSE related
library and utilities. This part of the `libecl` distribution can also be
built on Windows with Visual Studio (albeit with maaaany warnings) and
with MinGW.


## 3. Building `libecl`

CMake is the build system for `libecl`. The top level CMakeLists.txt file
is located in the top level directory of the repository, and this
CMakeLists.txt file includes individual CMakeLists.txt files for the
different libraries.

Building with CMake is performed like this:

1. Create a build directory, this can in principle be anywhere in the
filesystem. One level above the toplevel source directory is a practical choice.
2. Go to the build directory and invoke the command:
```
ccmake <path/to/directory/containing/CMakeLists.txt>
```
Go through several 'configure' steps with CMake and generate native build files.
3. Exit ccmake and invoke the native build system, i.e. ordinarily 'make' on
Linux.
4. Subsequent builds can be performed using just the native make command, as in
step 3.

### 3.1 CMake settings you might want to adjust

The main setting you should adjust is `BUILD_ERT` which is default to
`OFF`, i.e.  by default only the ECLIPSE related utilities will be
built. The build system has numerous configurations checks; the
ECLIPSE utilities should build on Windows, but to build all of `libecl` you
will need a Linux (Posix) system.


## 4. The code

The code is mainly a collection of libraries written in C.

### 4.1 The different libraries

The _provided libraries_ are:

* `libert_util`: This library is a collection of utilities of various sorts; if
C++ had been chosen as implementation language, most of these utilities could
probably be replaced by standard C++ classes.
* `libgeometry`: This is a very small geometry library; the main code is a small
implementantion of an alorithm to determine whether a point is inside a
polyhedron. The ECLIPSE library has some geometry related code which should be
moved here.
* `libwell`: This library will load well information from an ECLIPSE restart
file.  This is mainly for the purpose of visualization of the existing wells,
and can not be used to update or model the well configuration.
* `libecl`: This library will read and (partly) write the various binary ECLIPSE
files, including `GRID/EGRID`, summary, `INIT`, restart and `RFT` files. There
is also support for reading an writing grdecl formatted files, but there is no
support for general parsing of the _ECLIPSE_ input format.



### 4.2 General structure

The code is written in C, but conventions give a 'scent of object
orientation'. Most of the code is uses the following conventions:

* Every file 'xxx' implements a data type 'xxx_type' - this naming convention is
quite strong.
* All the structure definitions are in the source files, i.e. external scopes
must access the data of a structure through accessor functions.
* All functions which operate on a type 'xxx_type' take a pointer to xxx_type as
their first argument, the structure closely resemble the 'self' argument used
when implementing Python classes.
* Memory management is manual; however there are some conventions:
    * Functions allocating storage have _alloc_ as part of the name.
    * For all functions xxx_alloc() which allocate memory, there
should be a matching xxx_free() function to discard the objects.
    * Containers can optionally destroy their content if the content
is installed with a destructor.
* In `libert_util/src/type_macros.h` there is a macro based
'type-system' which is used to runtime check casts of (void *).


### 4.3 Python wrappers

Some of the code, in particular the ECLIPSE related functionality, has
been wrapped for usage in Python. Using these wrappers, it is quite
easy work with ECLIPSE files. The python wrappers are quite well
documented both in the directory python/docs and in the Python classes
themselves.


## 5. Tests

The `libecl` codebase has a small but increasing test coverage. The tests
are typically located in a per-library subdirectory tests/. The test
framework is based on ctest which basically works like this:

1. In the CMakeLists.txt file testing is enabled with `ENABLE_TESTING()`.
2. Tests are added with:
```python
add_test( test_name test_executable arg1 arg2 arg3 ... )
```

If the executable exits with status 0 the test has passed,
otherwise it has failed. The executable run by the test can
typically be an executable built as part of the solution, but can
in principle be an arbitrary executable like a dedicated test
runner or e.g. the Python interpreter.

### 5.1 Testing of C code

The main part of the testing infrastructure are small C applications
which are added like this:

```
add_executable( test_exe test_source.c )
target_link_libraries( test_exe lib )
add_test( test_name ${EXECUTABLE_OUTPUT_PATH}/test_exe commandline_arg1 commandline_arg2 )
```

Where the first two lines create the test executable in the normal
way, and the last line adds it as a test.

### 5.2 Testing of Python Code

In python/test there are several files with Python tests, these files
are executable files and they are invoked directly from the command
line. A limited number of the tests have been integrated in the ctest
system.

### 5.3 Test names

The tests in the cmake build system follow the naming convention of
the library regarding the functionality which they are testing: For
example, all tests for the libecl library use a name starting with
'ecl' and all tests for the tests for the config library are prefixed
by 'config'. The ctest options -R and -E can be used to include and
exclude tests based on their name

```
ctest -R ecl      # Run all tests containing the regular expression 'ecl'
ctest -E ecl      # Run all tests NOT containing the regular expression 'ecl'
```

### 5.4 Test labels

Using the cmake set_property() function it is possible to assign
labels to the test, and the -L and -LE options to ctest can be used to
limit which tests to run. A test can only have one label; in the
current ctest setup different labels are combined into one composite
label with a ":" separator, e.g.

```
set_property( TEST test_xxx PROPERTY LABELS StatoilData:Python)
```

will set the 'StatoilData' and 'Python' properties on test_xxx. The
labels currently available in the `libecl` test setup are:

   StatoilData: This implies that the test makes use of Statoil
      internal data. If you are work for the Bergen office of Statoil,
      you can read the the file test-data/README for instructions on
      how to make this data available.

      If you are not for Statoil in Bergen, you must pass the option:
      "-EL StatoilData" to ctest to skip all the tests which require
      Statoil internal data.

   StatoilBuild: There is one python test which makes use of Statoil
      internal configuration data, this test is labeled with
      StatoilBuild. If you want to run this test, you must set the
      cmake option ECL_LOCAL_TARGET to point to a file which contains
      these local configuration settings, e.g. where the ECLIPSE binary
      is installed.

   Python: This label is used to indicate that the test uses Python.

   LSF: This labels indicates that the test needs a working LSF
      environment to run.


### 5.5 ctest examples

```
ctest -L Statoil         # Run all tests labeled with Statoil - both
                         # StatoilData and StatoilBuild

ctest -EL "Statoil|LSF"  # Exclude all tests labeled with Statoil or LSF.
```
