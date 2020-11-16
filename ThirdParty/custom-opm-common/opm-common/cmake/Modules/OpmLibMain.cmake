# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# - Build an OPM library module
#
# This macro assumes that ${project} contains the name of the project,
# e.g. "opm-core", and that various variables that configures the module
# has been setup in advance.
#
# Customize the module configuration by defining these "callback" macros:
#
#	prereqs_hook    Do special processing before prerequisites are found
# fortran_hook    Determine whether Fortran support is necessary or not
#	sources_hook    Do special processing before sources are compiled
#	tests_hook      Do special processing before tests are compiled
#	files_hook      Do special processing before final targets are added


# don't write default flags into the cache, preserve that for user set values
include (AddOptions)
no_default_options ()

# Languages and global compiler settings
if(CMAKE_VERSION VERSION_LESS 3.8)
  message(WARNING "CMake version does not support c++17, guessing -std=c++17")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Various compiler extension checks
include(OpmCompilerChecks)

# print system information to better pinpoint issues from log alone
include (UseSystemInfo)
system_info ()

# very early try to print repo id (to pinpoint version if something goes wrong)
include (UseVCSInfo)
vcs_info ()

# print toolchain information to identify compilers with potential bugs
include (UseCompVer)
compiler_info ()
linker_info ()

# default settings: build static debug library
include (OpmDefaults)
opm_defaults (${project})
message (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# use tricks to do faster builds
include (UseFastBuilds)

# precompiled headers
include (UsePrecompHeaders)

# optimize full if we're not doing a debug build
include (UseOptimization)

# turn on all warnings; this must be done before adding any
# dependencies, in case they alter the list of warnings
include (UseWarnings)

# parallel programming
include (UseOpenMP)
find_openmp (${project})
include (UseThreads)
find_threads (${project})

# SuperLU is optional
option (USE_SUPERLU "Use SuperLU direct solvers" OFF)

# PETSc is optional
option (USE_PETSC "Use PETSc iterative solvers" OFF)

# static code analysis
include(UseStaticAnalysis)
setup_static_analysis_tools()

# callback hook to setup additional dependencies
if (COMMAND prereqs_hook)
	prereqs_hook ()
endif (COMMAND prereqs_hook)

# macro to set standard variables (INCLUDE_DIRS, LIBRARIES etc.)
include (OpmFind)
find_and_append_package_list_to (${project} ${${project}_DEPS})

# set aliases to probed variables
include (OpmAliases)

# remove the dependency on the testing framework from the main library;
# it is not possible to query for Boost twice with different components.
list (REMOVE_ITEM "${project}_LIBRARIES" "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")

# don't import more libraries than we need to
include (UseOnlyNeeded)

# put debug information into every executable
include (UseDebugSymbols)

# detect if Boost is in a shared library
include (UseDynamicBoost)

# needed for Debian installation scheme
include (GNUInstallDirs)

# Run conditional file hook
files_hook()

# this module contains code to figure out which files is where
include (OpmFiles)
opm_auto_dirs ()

# put libraries in lib/
opm_out_dirs ()

# identify the compilation units in the library; sources in opm/,
# tests files in tests/, examples in tutorials/ and examples/
opm_sources (${project})

# processing after base sources have been identified
if (COMMAND sources_hook)
	sources_hook ()
endif (COMMAND sources_hook)

# convenience macro to add version of another suite, e.g. dune-common
macro (opm_need_version_of what)
	string (TOUPPER "${what}" _WHAT)
	string (REPLACE "-" "_" _WHAT "${_WHAT}")
	list (APPEND ${project}_CONFIG_IMPL_VARS
		${_WHAT}_VERSION_MAJOR ${_WHAT}_VERSION_MINOR ${_WHAT}_VERSION_REVISION
		)
endmacro (opm_need_version_of suite module)

# use this hook to add version macros before we write to config.h
if (COMMAND config_hook)
	config_hook ()
endif (COMMAND config_hook)

# create configuration header which describes available features
# necessary to compile this library. singular version is the names that
# is required by this project alone, plural version transitively
# includes the necessary defines by the dependencies
include (ConfigVars)
list (APPEND ${project}_CONFIG_VARS ${${project}_CONFIG_VAR})

# write configuration variables to this file. note that it is a temporary.
# _CONFIG_IMPL_VARS are defines that are only written to config.h internal
# to this project; they are not exported to any installed files.
# TESTING_CONFIG_VARS is what's required by the unit tests, and is therefore
# added in an ad-hoc manner to avoid putting dependencies to it in the module
# requirement file. (it should be added if there is .h code that needs it)
message (STATUS "Writing config file \"${PROJECT_BINARY_DIR}/config.h\"...")
set (CONFIG_H "${PROJECT_BINARY_DIR}/config.h.tmp")
configure_vars (
	FILE  CXX  ${CONFIG_H}
	WRITE ${${project}_CONFIG_VARS}
	      ${${project}_CONFIG_IMPL_VARS}
	      ${TESTING_CONFIG_VARS}
	)

# call this hook to let it setup necessary conditions for Fortran support
if (COMMAND fortran_hook)
	fortran_hook ()
endif (COMMAND fortran_hook)

if (${project}_FORTRAN_IF)
	include (UseFortranWrappers)
	define_fc_func (
		APPEND ${CONFIG_H}
		IF ${${project}_FORTRAN_IF}
		)
endif (${project}_FORTRAN_IF)

# overwrite the config.h that is used by the code only if we have some
# real changes. thus, we don't have to recompile if a reconfigure is run
# due to some files being added, for instance
execute_process (COMMAND
	${CMAKE_COMMAND} -E copy_if_different ${CONFIG_H} ${PROJECT_BINARY_DIR}/config.h
	)

# compile main library; pull in all required includes and libraries
include (OpmCompile)
opm_compile (${project})

# installation target: copy the library together with debug and
# configuration files to system directories
include (OpmInstall)
if (COMMAND install_hook)
	install_hook ()
endif (COMMAND install_hook)
opm_install (${project})
message (STATUS "This build defaults to installing in ${CMAKE_INSTALL_PREFIX}")

# installation of CMake modules to help user programs locate the library
include (OpmProject)
opm_cmake_config (${project})

# routines to build satellites such as tests, tutorials and samples
include (OpmSatellites)

# example programs are found in the tutorials/ and examples/ directory
option (BUILD_EXAMPLES "Build the examples/ tree" ON)
if (BUILD_EXAMPLES)
	opm_compile_satellites (${project} examples "" "")
endif (BUILD_EXAMPLES)

opm_compile_satellites (${project} additionals EXCLUDE_FROM_ALL "")

# attic are programs which are not quite abandoned yet; however, they
# are not actively maintained, so they should not be a part of the
# default compile
opm_compile_satellites (${project} attic EXCLUDE_FROM_ALL "")

# infrastructure for testing
enable_testing ()
include (CTest)

# conditionally disable tests when features aren't available
macro (cond_disable_test name)
	if ((NOT DEFINED HAVE_${name}) OR (NOT HAVE_${name}))
		message (STATUS "${name} test disabled, since ${name} is not found.")
		string (TOLOWER "${name}" name_lower)
		get_filename_component (test_${name}_FILE "tests/test_${name_lower}.cpp" ABSOLUTE)
		list (REMOVE_ITEM tests_SOURCES "${test_${name}_FILE}")
	endif ((NOT DEFINED HAVE_${name}) OR (NOT HAVE_${name}))
endmacro (cond_disable_test name)

# use this target to run all tests, with parallel execution
cmake_host_system_information(RESULT TESTJOBS QUERY NUMBER_OF_PHYSICAL_CORES)
if(TESTJOBS EQUAL 0)
	set(TESTJOBS 1)
endif()
add_custom_target (check
	COMMAND ${CMAKE_CTEST_COMMAND} -j${TESTJOBS}
	DEPENDS test-suite
	COMMENT "Checking if library is functional"
	VERBATIM
	)

# special processing for tests
if (COMMAND tests_hook)
	tests_hook ()
endif (COMMAND tests_hook)

# make datafiles necessary for tests available in output directory
opm_data (tests datafiles "${tests_DIR}")
if(NOT BUILD_TESTING)
  set(excl_all EXCLUDE_FROM_ALL)
endif()
opm_compile_satellites (${project} tests "${excl_all}" "${tests_REGEXP}")

# use this target to check local git commits
add_custom_target(check-commits
                  COMMAND ${CMAKE_COMMAND}
                          -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                          -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
                          -P ${OPM_MACROS_ROOT}/cmake/Scripts/CheckCommits.cmake)

# generate documentation from source code with Doxygen;
# setup install target for this documentation
include (OpmDoc)
opm_doc (${project} ${doxy_dir})

### clean in-source builds ###
include (OpmDistClean)
opm_dist_clean (${project})

### emulate the with-xxx feature of autotools;
include (OpmKnown)

# make sure we rebuild if dune.module changes
configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/dune.module"
	"${CMAKE_CURRENT_BINARY_DIR}/dunemod.tmp"
	COPYONLY
	)

# make sure updated version information is available in the source code
include (UseVersion)

# update the cache for next run
write_back_options ()
