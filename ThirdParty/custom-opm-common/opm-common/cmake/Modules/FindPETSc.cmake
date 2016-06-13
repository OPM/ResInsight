# - Try to find Petsc lib
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(Petsc)
#
# Once done this will define
#
#  PETSC_FOUND - system has Petsc lib with correct version
#  PETSC_INCLUDE_DIRS - the Petsc include directory
#  PETSC_LIBRARIES   - the Petsc library.

# Copyright (c) 2006, 2007 Montel Laurent, <montel@kde.org>
# Copyright (c) 2008, 2009 Gael Guennebaud, <g.gael@free.fr>
# Copyright (c) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
# Redistribution and use is allowed according to the terms of the 2-clause BSD license.

# find out the size of a pointer. this is required to only search for
# libraries in the directories relevant for the architecture
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)

# if PETSC_ROOT is set, then this is the only place searched for petsc headers
# and includes
set(_no_default_path "")
if(PETSC_ROOT)
    set (_no_default_path "NO_DEFAULT_PATH")
endif()

# look for a system-wide BLAS library
set(PETSC_BLAS_LIBRARY "")
find_package(BLAS QUIET)
list(APPEND PETSC_BLAS_LIBRARY "${BLAS_LIBRARIES}")

# if BLAS wasn't found, look for it in PETSC_ROOT. Won't search if
# PETSC_BLAS_LIBRARY is set.
find_library(PETSC_BLAS_LIBRARY
    NAME "blas"
    PATH ${PETSC_ROOT}
    PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
    ${_no_default_path}
    )

# print message if there was still no blas found!
if(NOT BLAS_FOUND AND NOT PETSC_BLAS_LIBRARY)
  message(STATUS "BLAS not found but required for PETSc")
  return()
endif()

set(PETSC_LAPACK_LIBRARY "")
find_package(LAPACK QUIET)
list(APPEND PETSC_LAPACK_LIBRARY "${LAPACK_LIBRARIES}")

# if LAPACK wasn't found, look for it in PETSC_ROOT
find_library(PETSC_LAPACK_LIBRARY
    NAME "lapack"
    PATH ${PETSC_ROOT}
    PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
    ${_no_default_path}
    )

# print message if there was still no blas found!
if(NOT LAPACK_FOUND AND NOT PETSC_LAPACK_LIBRARY)
  message(STATUS "LAPACK not found but required for PETSc")
  return()
endif()

find_package(X11 QUIET)
if (X11_FOUND)
    list(APPEND PETSC_X11_LIBRARY "${X11_LIBRARIES}")
endif()

# these variables must exist. Since not finding MPI, both the header and the
# object file , may not be an error, we want the option of concatenating the
# empty variable onto the PETSC_LIBRARIES/INCLUDE_DIRS lists
set(PETSC_MPI_LIBRARY "")
set(PETSC_MPI_INCLUDE_DIRS "")

find_package(MPI)
if(MPI_FOUND)
    list(APPEND PETSC_MPI_LIBRARY "${MPI_LIBRARIES}")
    set(PETSC_MPI_INCLUDE_DIRS ${MPI_INCLUDE_PATH})

else(MPI_FOUND)
# if a system MPI wasn't found, look for PETSc's serial implementation. This
# won't be available if PETSc was compiled with --with-mpi=0, so not finding
# this won't be an error. This only needs to find the header, as the MPI
# implementation should be already be compiled into PETSc.
    message(STATUS "Could not find a system provided MPI. Searching for PETSc provided mpiuni fallback implementation.")
    find_path(PETSC_MPI_INCLUDE_DIRS
        NAMES "mpi.h"
        PATHS ${PETSC_ROOT}/include
        PATH_SUFFIXES "mpiuni"
        ${_no_default_path}
        )
endif(MPI_FOUND)

if(NOT PETSC_MPI_INCLUDE_DIRS)
    message(WARNING "Could not find any MPI implementation. If PETSc is compiled with --with-mpi=0 this is ok. Otherwise you will get linker errors or (possibly subtle) runtime errors. Continuing.")
    if(NOT USE_MPI)
        message("To build with MPI support, pass -DUSE_MPI=ON to CMake.")
    endif(NOT USE_MPI)
endif(NOT PETSC_MPI_INCLUDE_DIRS)

# only probe if we haven't a path in our cache
if (Petsc_ROOT)
    set (PETSC_ROOT "${Petsc_ROOT}")
endif (Petsc_ROOT)

find_path (PETSC_NORMAL_INCLUDE_DIR
    NAMES "petsc.h"
    PATHS ${PETSC_ROOT}
    PATH_SUFFIXES "include" "petsc"
    ${_no_default_path}
    )

list(APPEND PETSC_INCLUDE_DIR ${PETSC_NORMAL_INCLUDE_DIR})

# look for actual Petsc library
find_library(PETSC_LIBRARY
    NAMES "petsc-3.4.3" "petsc-3.4.4" "petsc" 
    PATHS ${PETSC_ROOT}
    PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
    ${_no_default_path}
    )

if(NOT PETSC_LIBRARY)
  message(STATUS "Could not find the PETSc library")
  return()
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Petsc DEFAULT_MSG PETSC_INCLUDE_DIR PETSC_LIBRARY)
mark_as_advanced(PETSC_INCLUDE_DIR PETSC_LIBRARY)

# if both headers and library are found, store results
if(PETSC_FOUND)
    set(PETSC_INCLUDE_DIRS ${PETSC_INCLUDE_DIR})
    list(APPEND PETSC_INCLUDE_DIRS ${PETSC_MPI_INCLUDE_DIRS})

    set(PETSC_LIBRARIES ${PETSC_LIBRARY})

    list(APPEND PETSC_LIBRARIES ${PETSC_BLAS_LIBRARY})
    list(APPEND PETSC_LIBRARIES ${PETSC_LAPACK_LIBRARY})
    list(APPEND PETSC_LIBRARIES ${PETSC_X11_LIBRARY})
    list(APPEND PETSC_LIBRARIES ${PETSC_MPI_LIBRARY})
endif()
