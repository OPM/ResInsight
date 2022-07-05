# - Initialize project-specific variables
#
# This will read the dune.module file for project information and
# set the following variables:
#
#	project                     From the Module: field
#	${project}_NAME             Same as above
#	${project}_DESCRIPTION      From the Description: field
#	${project}_VERSION_MAJOR    From the Version: field
#	${project}_VERSION_MINOR    From the Version: field also
#
# This module should be the first to be included in the project,
# because most of the others (OpmXxx.cmake) use these variables.

# for CMake >= 3.0, we need to change a few policies:
#
#   - CMP0026 to allow access to the LOCATION target property
#   - CMP0048 to indicate that we want to deal with the *VERSION*
#     variables ourselves
#   - CMP0064 to indicate that we want TEST if conditions to be evaluated
#   - CMP0074 to indicate that <PackageName>_ROOT can be used to find package
#             config files
macro(OpmSetPolicies)
  if (POLICY CMP0026)
    # Needed as we query LOCATION in OpmCompile.cmake and OpmSatellites.cmake
    cmake_policy(SET CMP0026 OLD)
  endif()

  if (POLICY CMP0048)
    # We do not set version. Hence NEW should work and this can be removed later
    cmake_policy(SET CMP0048 NEW)
  endif()

  if(POLICY CMP0064)
    cmake_policy(SET CMP0064 NEW)
  endif()

  # set the behavior of the policy 0054 to NEW. (i.e. do not implicitly
  # expand variables in if statements)
  if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
  endif()

  # set the behavior of policy 0074 to new as we always used <PackageName>_ROOT as the
  # root of the installation
  if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
  endif()

  # include special
  if (CMAKE_VERSION VERSION_LESS "2.8.3")
    message (STATUS "Enabling compatibility modules for CMake 2.8.3")
    list (APPEND CMAKE_MODULE_PATH "${OPM_MACROS_ROOT}/cmake/Modules/compat-2.8.3")
  endif (CMAKE_VERSION VERSION_LESS "2.8.3")

  if (CMAKE_VERSION VERSION_LESS "2.8.5")
    message (STATUS "Enabling compatibility modules for CMake 2.8.5")
    list (APPEND CMAKE_MODULE_PATH "${OPM_MACROS_ROOT}/cmake/Modules/compat-2.8.5")
  endif (CMAKE_VERSION VERSION_LESS "2.8.5")

  if (CMAKE_VERSION VERSION_LESS "2.8.7")
    message (STATUS "Enabling compatibility modules for CMake 2.8.7")
    list (APPEND CMAKE_MODULE_PATH "${OPM_MACROS_ROOT}/cmake/Modules/compat-2.8.7")
  endif (CMAKE_VERSION VERSION_LESS "2.8.7")
endmacro()


# helper macro to retrieve a single field of a dune.module file
macro(OpmGetDuneModuleDirective field variable contents)
  string (REGEX MATCH ".*${field}:[ ]*([^\n]+).*" ${variable} "${contents}")
  string (REGEX REPLACE ".*${field}:[ ]*([^\n]+).*" "\\1" "${variable}" "${${variable}}")
  string (STRIP "${${variable}}" ${variable})
endmacro()

function (OpmInitProjVars)
  # locate the "dune.module" file
  set (DUNE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dune.module")

  # read this file into a variable
  file (READ "${DUNE_MODULE_PATH}" DUNE_MODULE)

  # read fields from the file
  OpmGetDuneModuleDirective ("Module" project "${DUNE_MODULE}")
  OpmGetDuneModuleDirective ("Description" description "${DUNE_MODULE}")
  OpmGetDuneModuleDirective ("Version" version "${DUNE_MODULE}")
  OpmGetDuneModuleDirective ("Label" label "${DUNE_MODULE}")

  # parse the version number
  set (verno_regex "^([0-9]*)\\.([0-9]*).*\$")
  string (REGEX REPLACE "${verno_regex}" "\\1" major "${version}")
  string (REGEX REPLACE "${verno_regex}" "\\2" minor "${version}")

  # return these variables
  set (project "${project}" PARENT_SCOPE)
  set (${project}_NAME "${project}" PARENT_SCOPE)
  set (${project}_DESCRIPTION "${description}" PARENT_SCOPE)
  set (${project}_VERSION_MAJOR "${major}" PARENT_SCOPE)
  set (${project}_VERSION_MINOR "${minor}" PARENT_SCOPE)  
  set (${project}_LABEL "${label}" PARENT_SCOPE)
endfunction ()

macro (OpmInitDirVars)
  # these are the most common (and desired locations)
  set (${project}_DIR "opm")
  set (doxy_dir "doc/doxygen")

  # but for backward compatibility we can override it
  if (COMMAND dir_hook)
	dir_hook ()	
  endif (COMMAND dir_hook)
endmacro ()

OpmInitProjVars ()
OpmInitDirVars ()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
include(OpmPackage)

# if we are backporting this release to a system which already have an
# earlier version, set this flag to have everything scoped into a directory
# which incorporates the label of the release. this is done by interjecting
# the ${project}_VER_DIR into the installation path.
option (USE_VERSIONED_DIR "Put files in release-specific directories" OFF)
set (${project}_SUITE "opm")
if (USE_VERSIONED_DIR)
  set (${project}_VER_DIR "/${${project}_SUITE}-${${project}_LABEL}")
else ()
  set (${project}_VER_DIR "")
endif ()

# parallel computing must be explicitly enabled
# This needs to be in OpmInit as prereqs is called before OpmLibMain is included.
option (USE_MPI "Use Message Passing Interface for parallel computing" ON)
if (NOT USE_MPI)
  set (CMAKE_DISABLE_FIND_PACKAGE_MPI TRUE)
endif ()

# Compiler standard version needs to be requested here as prereqs is included
# before OpmLibMain and some tests need/use CXX_STANDARD_VERSION (e.g. pybind11)
# Languages and global compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# quadmath must be explicitly enabled
# This needs to be in OpmInit as prereqs is called before OpmLibMain is included.
option (USE_QUADMATH "Search for high precision floating point library (normally not used)" ON)
if (NOT USE_QUADMATH)
  set (CMAKE_DISABLE_FIND_PACKAGE_QuadMath TRUE)
endif ()

option (USE_SUPERLU "Use SuperLU direct solvers for AMG (if umfpack is not found)" ON)
if (NOT USE_SUPERLU)
  set (CMAKE_DISABLE_FIND_PACKAGE_SuperLU TRUE)
endif ()
