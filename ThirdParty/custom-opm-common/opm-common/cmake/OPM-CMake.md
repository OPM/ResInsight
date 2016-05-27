OPM Build System
================

This is the documentation for the build system used in various OPM modules.
In the following, `xxx` is used as a placeholder for the project name (e.g.
"core").

Unlike traditional CMake files which is highly imperative, OPM projects
sets up declarative lists of prerequisites and content and rely on convention
and pre-made modules to do the build. Its goal is to replace but be
compatible with the old autotools-based system.

## Terminology

In the build system to following abstract locations are referred to:

<table><thead><tr><th>Location<th>Description<tbody>
<tr>
<td>	Source tree
<td>
This is where the source files are located. Usually this directory is created
by a `git clone`. You edit files in this directory. The build system on the
other hand will never touch these files; they could be read-only for that
matter. It should be located on a disk that is backed up. The source trees
for various OPM modules should be siblings to eachother.

<tr>
<td>	Build tree
<td>
This is where you do `make` (or `ninja`), and the compiler/linker will put
its output here. It may be the same directory as the source tree (which is
then called an "in-tree build"). However, it is recommended that you keep
it separate. Do `make clean && make distclean` to remove all files that
are created by the build (if you put it in the source directory).
You don't need to backup these files (since they can be generated from the
source); instead this directory should be located somewhere with fast
write access. The build trees for various OPM modules should be siblings
(unless they are subdirectories in their source trees).

<tr>
<td>	Installation tree
<td>
This is where the build system will put all the final libraries and headers
when running `make install`.
You can specify another location for the installation tree by setting the
CMake variable `CMAKE_INSTALL_PREFIX` on the command line (or use `--prefix=`
with the configure script). Notice that the portion of this path which
will become the new filesystem root should be specified with the environment
variable `DESTDIR`.

</table>

## Use Cases

This section lists some common use cases for adding new code to a project
with respect to the build system, and lists the steps that must be undertaken
to do the necessary modifications.

### Adding a Translation Unit

1. Put the file in a sub-directory of `opm/xxx`.

2. Add the file name to the `MAIN_SOURCE_FILES` list in `CMakeLists_files.txt`.
   Please keep this list sorted.

3. If you are adding new interfaces that will be used by client code, add the
   header to the `PUBLIC_HEADER_FILES`. Note that any `_impl` headers containing
   template implementations must also be included.

### Adding a Prerequisite

1. Add the name of the prerequisite module to the `opm-xxx_DEPS` list in the file
   `cmake/Modules/opm-xxx-prereqs.cmake`, where xxx is a placeholder for the module
   name of your CMake project.

2. If you need any CMake configuration defines available in your public _headers_,
   add these to the `opm-xxx_CONFIG_VAR` list in the same file. Please refrain
   from this practice as it imposes a requirement on the clients of your code to
   gather the same configuration information and make it available in the units
   which uses your headers.

### Adding a Unit Test

1. Put the source code in a single translation unit in directory `tests`. The
   name of this unit should start with `test_`.

2. Put any datafiles this code rely on in the same directory. The code should
   assume that such datafiles are available in the current directory the program
   is running from. The code should not write to these files, but rather make
   a copy if it needs to modify them.

3. Add the file name to the `TEST_SOURCE_FILES` list in `CMakeLists_files.txt`.

4. Add the datafiles to the `TEST_DATA_FILES` list in the same files. The
   files will be copied from the source tree into the target tree.

### Adding a New Utility Program

1. Put the source code of the utility in the `examples` directory.

2. Add the name of the translation unit to the `PROGRAM_SOURCE_FILES` list
   in `CMakeLists_files.txt`.
   
### Creating a New Module

1. Copy the directory `cmake/` and all sub-directories from opm-core. This
   directory contains the common build system, and should ideally be identical
   across OPM projects. Also copy the file `configure` in the project root.

2. Create project-specific files using those from another project as a template.
   The list of project specific files is in the section
   [Modules Reference](#project-specific-files) below.

3. Create a new file `cmake/Modules/opm-xxx-prereqs.cmake`, using one of the
   existing ones as a template.

4. Optionally, create a new file `cmake/Modules/Findopm-xxx.cmake`, using one
   of the existing ones as a template.

## Options

These options regulate the behaviour of the build system. In addition to these
options, you can also set standard CMake options, or options for the
prerequisites, which is not documented here. If you run the configure script
with the `--help` option, it will print a text of what the options are called
when using the autotools-compatible wrapper.

<table><thead><tr><th>Option<th>Description<tbody>
<tr>
<td>	BUILD_EXAMPLES
<td>
Include the examples when doing a build. Whenever you change something
in the library, however small, all the examples will also be rebuilt.
Default is ON.

<tr>
<td>	BUILD_TESTING
<td>
Include the unit tests when doing a build. Whenever you change something
in the library, however small, all the unit tests will also be rebuilt.
Default is ON.

<tr>
<td>	PRECOMPILE_HEADERS
<td>
Precompile common headers into a binary blob which is loaded on further
compilations. If your compiler supports this, it usually reduces build
time. It does not affect the running time of the code. Default is OFF.

<tr>
<td>	SIBLING_SEARCH
<td>
Search for OPM/DUNE prerequisites in sibling directories of the build
tree. Default is ON.

<tr>
<td>	SUITESPARSE_USE_STATIC
<td>
Link SuiteSparse/UMFPack statically. Using this option will enable you
to build an executable which has no external dependencies. The default
is to use shared libraries if those are available.

<tr>
<td>	SYSTEM_DEBUG
<td>
Put debug symbols in the system debug directory (`/usr/lib/debug`) as
this seems to be the only place which is searched by default by GDB.
Default is OFF, as it requires that you have write access to that
directory. Note that if you are doing a system installation (set
CMAKE_INSTALL_PREFIX=/usr), then the libraries will be put in this
location irregardless of this option.

<tr>
<td>	USE_MPI
<td>
Enable the code to use MPI for parallelization. Default is OFF.
Note: It is important that OPM and DUNE modules is either all
compiled with MPI support or that none is. The build system will
attempt to recognize inconsistencies.

<tr>
<td>	USE_OPENMP
<td>
Enable the code to use OpenMP for parallelization. Default is ON.

<tr>
<td>	USE_RUNPATH
<td>
Remember the directories from where the prerequisites were found
when building. Default is ON, which enables you to run without
setting PATH all over the place in your build directories. When
creating an installation package, this should be set off.

<tr>
<td>	USE_UNDERSCORING
<td>
Assume that Fortran externals have an underscore suffix instead
of checking this with an actual compiler. If you set this option,
you can use Fortran libraries (notably BLAS and LAPACK) without
having a Fortran compiler installed. The default is OFF.

<tr>
<td>	USE_VERSIONED_DIR
<td>
Put libraries in a directory which includes the label of the project,
e.g. `/usr/lib/opm-core/2013.10`. Default is OFF.

<tr>
<td>	WITH_NATIVE
<td>
Optimize for the instruction set of the build machine. This is
a good idea if you are building the library on the same machine
as you will be using the library. Default is ON.

<tr>
<td>	WHOLE_PROG_OPTIM
<td>
Perform an extra round of optimization when linking the program.
(Usually the compiler only optimizes within the translation unit).
If your compiler supports this, it usually leads to a faster runtime.
Default is OFF.

</table>
   
## Modules Reference

### Project-specific Files

All of these files are in the project root.

<table><thead><tr><th>File<th>Description<tbody>
<tr>
<td>	CMakeLists.txt
<td>
Project-specific customizations to the build, such as filtering out source
files based on the availability of prerequisites, or adding configuration
variables only the implementation depends on.
Prefer to do customizations in the hooks available to this file rather than
adding ad-hoc code to the build system itself, to keep the `cmake/` directory
unified across projects.

<tr>
<td>	CMakeLists_files.txt
<td>
List of all compilation modules in the project, test cases and public
headers that should be installed in the distribution package. The contents
of these lists are distributed to project-specific variables by the build
system.

<tr>
<td>	CTestConfig.cmake
<td>
Settings for submitting result of tests to CDash. The default is setup
to submit to [the official CDash panel](http://www.opm-project.org/CDash/)
and does not need to be changed if your module has a panel there.

<tr>
<td>	dune.module
<td>
Information about the project such as name, release label, link version
and maintainer. Also specify dependencies to other OPM/DUNE-projects so
that dunecontrol can build in correct order. (Note that the full list of
dependencies is taken from opm-xxx-prereqs.cmake and not from here).
Since this file must be present before the build starts (for dunecontrol),
the version information is kept here.

</table>

### Project Modules

These modules contains the dependency information for this project, so
the build system can set up the prerequisite list correctly and probe
for other modules automatically. (This replaces hard-coded calls to
find_library in the CMakeLists.txt file).

<table><thead><tr><th>File (.cmake)<th>Description<tbody>
<tr>
<td>	xxx-prereqs
<td>
List prerequisite modules and defines used in public headers. Each module
must have such a "declaration", and this must be made available to every
other projects as well (which is why this is located in `cmake/Modules`).

<tr>
<td>	Findxxx
<td>
CMake modules which locates module `xxx` in the system directories. As
the `opm-xxx-config.cmake` is made available together with the libraries
and headers, these modules are not really needed (for OPM modules).

</table>

### Generated Files

These files are generated by the build system and exists in the _build_ tree,
not in the source tree. They are documented here to make developers aware of
their role in the build system.

<table><thead><tr><th>File<th>Description<tbody>
<tr>
<td>	config.h
<td>
Settings variables which the build system has configured and make available
to the source code. This file is **not** installed amongst the headers, so
you should never include this in a public header, even if you need the value
in one of these variables. Instead, you must rely on the client code to
configure this variable for you.

<tr>
<td>	opm-xxx.pc
<td>
pkg-config information file to locate the **build** tree. This is used by
the autotools build files, but can also be handy when manually building
small test programs for which you don't generate an own build system.

<tr>
<td>	opm-xxx-config.cmake
<td>
CMake information file to locate the **build** tree. This file is imported
when this project is set up as a prerequisite.

<tr>
<td>	opm-xxx-install.pc
<td>
pkg-config information file to locate the **installation** tree. It is
the same as `opm-xxx.pc` except that the paths are switched. When the
project is installed, this file is installed instead (under `lib/pkgconfig`
relative to the installation root). This directory should hence be put
in the search path to pkg-config to use the installed package. Before
installation, this file is worthless and should not be included, because
it does not refer to the build tree at all. (Be careful not to mix
the build and the installation tree).
Notice that the build system will forward a bunch of public definitions
which should be available to compile code referring to this library.

<tr>
<td>	opm-xxx-install.cmake
<td>
CMake information file to locate the **installation** tree. It is the
same as `opm-xxx-config.cmake` except that the paths are switched. When
the project is installed, this file is installed instead (under `share/cmake`
relative to the installation root).

<tr>
<td>	opm-xxx-config-version.cmake
<td>
CMake wants to include this into the build _before_ it is determined whether
the library was found successfully (depending on the version number perhaps),
so this information is put in its own file. Since it is the same for the
build tree and the installation tree, it is shared in both those locations.

</table>

### Utility Modules

These modules consists of useful routines which is not OPM-specific and
that could be used in any projects. They don't depend on any other parts
of the build system.

<table><thead><tr><th>File (.cmake)<th>Description<tbody><tr>
<td>	AddOptions
<td>
Functions to add options to compiler command line (e.g. 'CXXFLAGS').
This macro can add options to more than one language and/or configuration
at a time, and also automatically removes duplicates.

<tr>
<td>	ConfigVars
<td>
Functions to write values of selected variables to `config.h`. The
advantage of using this compared to a template file, is that other
modules can add their own variables to be written (a project doesn't
need to know which variables a prerequisite wants to have in config.h).

<tr>
<td>	DuneCompat
<td>
Modify `Makefile` so dunecontrol can infer source directory from it.
dunecontrol infers the source location of prerequisites from textual
analysis of the Makefile in their build tree. (dunecontrol cannot build
with Ninja anyway, so that is not a problem).

<tr>
<td>	Duplicates
<td>
Functions to remove duplicate values from a list of libraries, which
must always be done from the beginning in order to link correctly.

<tr>
<td>	LibtoolArchives
<td>
Write .la file which will make libtool find our library. This enables
users of our library to use libtool even if we did not do so ourselves.
	
</table>

### Build System Modules

These are the modules which comprises the OPM-specific parts of the
build system. The overall flow through the stages of the build is best
captured by reading through the `OpmLibMain.cmake` module.

<table><thead><tr><th>File (.cmake)<th>Description<tbody>
<tr>
<td>	configure
<td>
Wrapper script which emulates an autotools front-end, making the build
system usable with dunecontrol. There is one in the project root directory
which just forwards everything to the main script in `cmake/Scripts`.

<tr>
<td>	OpmAliases
<td>
Copy variables which are probed by our find modules to the names which
are expected by DUNE.

<tr>
<td>	OpmCompile
<td>
Set up a compilation target for the library itself. It is assumed that
each OPM project build exactly one library file containing all its code.
The files and compiler options are passed through the project variables
(see the section [Variables Reference](#variables-reference) below).

<tr>
<td>	OpmDefaults
<td>
If optimization and debugging settings are not given on the command line,
supply a set of reasonable defaults for the detected platform and
compiler.

<tr>
<td>	OpmDistClean
<td>
Add a target (`make distclean`) to the build system which can remove the
build files themselves from the build directory tree.

<tr>
<td>	OpmDoc
<td>
Add target for building documentation, primarily Doxygen class reference
from the code.

<tr>
<td>	OpmFiles
<td>
Load list of files from `CMakeLists_files.txt` and put into the applicable
variables.

<tr>
<td>	OpmGrid
<td>
Adds the grid type selection code to config.h which is needed by dune-grid
if you want to set up a default grid. This is currently not needed by any
OPM project, and is provided only for porting client projects which previously
used this functionality from the autotools version.

<tr>
<td>	OpmInit
<td>
Read the version information and project name from `dune.module`.

<tr>
<td>	OpmInstall
<td>
Setup installation of the main library, public headers and debug symbols.

<tr>
<td>	OpmKnown
<td>
Marks as "used" all configuration variables which is used only by some of
the OPM projects, so they don't generate warnings in the rest of them.

<tr>
<td>	OpmLibMain
<td>
Driver module for the build process. First reads the list of prerequisites
and options, then set up the compiles and installations.

<tr>
<td>	OpmProject
<td>
Set up pkg-config and CMake information files (see [Generated Files]
(#generated-files)) for this project, based on configuration.

<tr>
<td>	OpmSatellites
<td>
Build test programs and examples for a library that is bundled in the project.
</table>

### Wrapper Modules

These modules wrap the CMake `find_library` function and adds the information
retrieved from the imported prerequisite to module-specific variables, so that
these can be added to the build in batch afterwards.

<table><thead><tr><th>File (.cmake)<th>Description<tbody>
<tr>
<td>	OpmFind
<td>
Wrapper around `find_package`. Searches in various locations relative to this
project as well as in the system directories for a CMake module which can
locate the package. If it is found, adds the project variables (see
[Variables Reference](#variables-reference)) for this project into this one,
for instance include and library directories are added to the compile and link
command-line for this project.

<tr>
<td>	OpmPackage
<td>
Typical way of finding an OPM package; searches for the header and library,
and tries to compile a test program. This is the general implementation of
a CMake find module, and is used to locate those of the prerequisites that
fits the pattern.

</table>

### Configuration Modules

These are modules for altering the compiler and/or linker option in some way,
or get information from the system. They are not tied to the OPM projects.

<table><thead><tr><th>File (.cmake)<th>Description<tbody>
<tr>
<td>	UseCompVer
<td>
Get the version of GCC that is used to compile the project. This is used in
other modules to enable features that are known to exist/work only in certain
versions of GCC.

<tr>
<td>	UseDebugSymbols
<td>
Set up the compile to generate debug symbols for the code. This is done also
if a release build was requested, to be able to do post-mortem debugging of
production code. (The debug symbols does not inhibit optimization).

<tr>
<td>	UseDuneVer
<td>
Retrieve the version of DUNE which is available.

<tr>
<td>	UseDynamicBoost
<td>
Determine if Boost is linked statically or dynamically (shared). This is
necessary to know for the unit tests.

<tr>
<td>	UseFastBuilds
<td>
Enable certain techniques which is known to speed up the build itself.

<tr>
<td>	UseFortranWrappers
<td>
Provide a macro for declaration of external symbols which is located in
Fortran libraries. It is not necessary to have a Fortran compiler present
to use this macro.

<tr>
<td>	UseMultiArch
<td>
Check if the system uses the multi-arch scheme for organizing libraries
(currently derivatives of Debian do this).

<tr>
<td>	UseOnlyNeeded
<td>
Only link to libraries which is actually used by the project. Some
platforms provide "under-linked" libraries (they need other libraries
but doesn't state so explicitly, but rather imply that the executable
must link to these itself), and this is also handled.

<tr>
<td>	UseOpenMP
<td>
Add OpenMP features to the build. Since OpenMP is activated by pragmas
in the code, compiler options instead of libraries are needed.

<tr>
<td>	UseOptimization
<td>
Compile with more extensive optimization that what is the default in
CMake.

<tr>
<td>	UsePrecompHeaders
<td>
Set up precompiled headers if the project has a `opm/xxx/opm-xxx-pch.hpp`
header. Due to problems across various compilers, this is currently an
opt-in feature.

<tr>
<td>	UseSystemInfo
<td>
Retrieve information about the system the build is performed on. This is
printed in the configuration log and can be helpful to troubleshoot
problems from error reports.

<tr>
<td>	UseVCSInfo
<td>
Retrieve information about which Git revision is compiled. This is useful
to figure out which version an error report refers to.

<tr>
<td>	UseVersion
<td>
Add version information for this project into the library binary, making
it available for query at runtime.

<tr>
<td>	UseWarnings
<td>
Enable a more extensive set of warnings to be reported by the compiler
than what is the default in CMake.

</table>

## Variables Reference

The build system will setup variables with names of the pattern `xxx_YYY`
where xxx is the project name (here including the suite; e.g. "opm-core")
and yyy is the suffix in the list below. The project name is used verbatim,
i.e. there is no translation of dashes and case ("opm-core" and not "OPM_CORE").

<table><thead><tr><th>Suffix<th>Description<tbody>
<tr>
<td>	_DEFINITIONS
<td>
Macro defines (of the type `-DFOO`) that needs to be added to the compile
of translation units contained in this library. This also includes defines
that must be present to headers which is included by this library.

<tr>
<td>	_CONFIG_VARS
<td>
Defines which should be present in `config.h` of the project which
includes this library (client code). Only the names of the variables
are listed here; the actual values must be found by the configuration
script of the client.

<tr>
<td>	_CONFIG_IMPL_VARS
<td>
Defines which should be present in `config.h` but is only used by
the internal code of the project itself. Use this list to get
defines without imposing a requirement on client code to also probe
for values.

<tr>
<td>	_INCLUDE_DIR
<td>
Directory where the public headers of this project are stored.

<tr>
<td>	_INCLUDE_DIRS
<td>
List of include directories that must be on the compiler search
path to compile code which uses this project. In addition to the
headers of this project itself, it also includes the transitive
closure of paths for all prerequisites as well.

<tr>
<td>	_LABEL
<td>
Currently for OPM projects, this follows a pattern of `YYYY.MM`
where YYYY is the year of the release and MM is the month. This
gives information to humans about how up to date this instance
of the library is (but doesn't provide a way to check for
compatibility, which is why the VERSION alternatives exist).

<tr>
<td>	_LIBRARY
<td>
Name and path of the binary to link with.

<tr>
<td>	_LIBRARIES
<td>
Full list of the library of both this project, and all its
prerequisites, that need to be included in the link. I.e. the
client code should only include the transitive list from its
immediate prerequisites and not know about the full dependency
graph.

<tr>
<td>	_LIBRARY_DIRS
<td>
Directories that should be added to the linker search path when
including this library.

<tr>
<td>	_LINKER_FLAGS
<td>
Flags that must be added to the link when including this library.

<tr>
<td>	_SOURCES
<td>
List of source files contained in this project. This enables libraries
to be distributed in source form (e.g. CJSON and TinyXML) and linked
directly into the project.

<tr>
<td>	_TARGET
<td>
Name of the library which is generated by this project. CMake and
autotools do not like library names which contains dashes, so they
are stripped out. By using a macro for this we are guaranteed uniform
translation.

<tr>
<td>	_VERSION
<td>
Textual concatenation of all components of the version number (see below)
with a dot inbetween. This form of version number can be compared using
CMake VERSION_{LESS|EQUAL|GREATER} operators. 

<tr>
<td>	_VERSION_MAJOR
<td>
Major version of the library. If the major versions doesn't match, then
compatibility with existing code cannot be reckoned.

<tr>
<td>	_VERSION_MINOR
<td>
Minor version of the library. Libraries with newer minor version can
have more features, but should be able to run old code.

<tr>
<td>	_VERSION_REVISION
<td>
Micro version of the library. This number is generally incremented
whenever bugfixes or performance improvements are made.
</table>
