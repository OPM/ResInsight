# - Try to find Eigen3 lib
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(Eigen3 3.1.2)
# to require version 3.1.2 or newer of Eigen3.
#
# Once done this will define
#
#  EIGEN3_FOUND - system has eigen lib with correct version
#  EIGEN3_INCLUDE_DIR - the eigen include directory
#  EIGEN3_VERSION - eigen version

# Copyright (c) 2006, 2007 Montel Laurent, <montel@kde.org>
# Copyright (c) 2008, 2009 Gael Guennebaud, <g.gael@free.fr>
# Copyright (c) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
# Redistribution and use is allowed according to the terms of the 2-clause BSD license.

if(NOT Eigen3_FIND_VERSION)
  if(NOT Eigen3_FIND_VERSION_MAJOR)
    set(Eigen3_FIND_VERSION_MAJOR 2)
  endif(NOT Eigen3_FIND_VERSION_MAJOR)
  if(NOT Eigen3_FIND_VERSION_MINOR)
    set(Eigen3_FIND_VERSION_MINOR 91)
  endif(NOT Eigen3_FIND_VERSION_MINOR)
  if(NOT Eigen3_FIND_VERSION_PATCH)
    set(Eigen3_FIND_VERSION_PATCH 0)
  endif(NOT Eigen3_FIND_VERSION_PATCH)

  set(Eigen3_FIND_VERSION "${Eigen3_FIND_VERSION_MAJOR}.${Eigen3_FIND_VERSION_MINOR}.${Eigen3_FIND_VERSION_PATCH}")
endif(NOT Eigen3_FIND_VERSION)

macro(_eigen3_check_version)
  file(READ "${EIGEN3_INCLUDE_DIR}/Eigen/src/Core/util/Macros.h" _eigen3_version_header)

  string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen3_world_version_match "${_eigen3_version_header}")
  set(EIGEN3_WORLD_VERSION "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen3_major_version_match "${_eigen3_version_header}")
  set(EIGEN3_MAJOR_VERSION "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen3_minor_version_match "${_eigen3_version_header}")
  set(EIGEN3_MINOR_VERSION "${CMAKE_MATCH_1}")

  set(EIGEN3_VERSION ${EIGEN3_WORLD_VERSION}.${EIGEN3_MAJOR_VERSION}.${EIGEN3_MINOR_VERSION})
  if(${EIGEN3_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})
    set(EIGEN3_VERSION_OK FALSE)
  else(${EIGEN3_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})
    set(EIGEN3_VERSION_OK TRUE)
  endif(${EIGEN3_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})

  if(NOT EIGEN3_VERSION_OK)

    message(STATUS "Eigen3 version ${EIGEN3_VERSION} found in ${EIGEN3_INCLUDE_DIR}, "
                   "but at least version ${Eigen3_FIND_VERSION} is required")
  endif(NOT EIGEN3_VERSION_OK)
endmacro(_eigen3_check_version)

# only probe if we haven't a path in our cache
if (NOT EIGEN3_INCLUDE_DIR)

  # allow Eigen3_ROOT to be used in addition to EIGEN3_ROOT
  if (Eigen3_ROOT)
	set (EIGEN3_ROOT "${Eigen3_ROOT}")
  endif (Eigen3_ROOT)

  # if the _ROOT is specified, then look *only* there; don't allow any
  # other version to be swapped in to substitute; if not specified, then
  # go search usual locations
  if (EIGEN3_ROOT)
	# if we are given the path to a "build" tree (meaning somewhere Eigen3
	# has been configured), then use the eigen3.pc file to figure out the
	# name of the *real* root directory
	if (EXISTS "${EIGEN3_ROOT}/CMakeCache.txt")
	  # get the cache entry that tells use the source tree location
	  set (_regex "Eigen_SOURCE_DIR:STATIC=\(.*\)")
	  file (STRINGS
		"${EIGEN3_ROOT}/CMakeCache.txt"
		EIGEN3_SOURCE_TREE
		REGEX "${_regex}"
		)
	  # trim away the key definition, be left with the value
	  if (EIGEN3_SOURCE_TREE)
		string (REGEX REPLACE
		  "${_regex}"
		  "\\1"
		  EIGEN3_SOURCE_TREE
		  "${EIGEN3_SOURCE_TREE}"
		  )
	  # if something doesn't look as expected, abort and search in _ROOT
	  else ()
		set (EIGEN3_SOURCE_TREE "${EIGEN3_ROOT}")
	  endif ()
	else ()
	  set (EIGEN3_SOURCE_TREE "${EIGEN3_ROOT}")
	endif ()

	find_path (EIGEN3_INCLUDE_DIR
	  NAMES signature_of_eigen3_matrix_library
	  PATHS ${EIGEN3_SOURCE_TREE}
	  PATH_SUFFIXES eigen3 include/eigen3 eigen include/eigen
	  NO_DEFAULT_PATH
	  )
  else (EIGEN3_ROOT)
	# assume that if there is a sibling directory to our project which
	# is called eigen3, there is a newer version located there, or that
	# it may have been checked out next to the build directory
	find_path(EIGEN3_INCLUDE_DIR
	  NAMES signature_of_eigen3_matrix_library
	  HINTS ${CMAKE_SOURCE_DIR}/../
	        ${PROJECT_SOURCE_DIR}/../
	        ${CMAKE_INSTALL_PREFIX}/include
	        ${KDE4_INCLUDE_DIR}
      PATH_SUFFIXES eigen3 eigen
    )
  endif (EIGEN3_ROOT)
endif (NOT EIGEN3_INCLUDE_DIR)

  if(EIGEN3_INCLUDE_DIR)
    _eigen3_check_version()
  endif(EIGEN3_INCLUDE_DIR)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Eigen3 DEFAULT_MSG EIGEN3_INCLUDE_DIR EIGEN3_VERSION_OK)

  mark_as_advanced(EIGEN3_INCLUDE_DIR)


