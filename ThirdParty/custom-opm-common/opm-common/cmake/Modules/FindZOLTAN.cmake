# -*-cmake-*-
#
# Try to find the libzoltan graph partioning library
#
# Once done, this will define:
#
#  ZOLTAN_FOUND        - system has the libzoltan graph partioning library
#  HAVE_ZOLTAN         - like ZOLTAN_FOUND, but for the inclusion in config.h
#  ZOLTAN_INCLUDE_DIR  - incude paths to use libzoltan
#  ZOLTAN_LIBRARIES    - Link these to use libzoltan

set(ZOLTAN_SEARCH_PATH "/usr" "/usr/local" "/opt" "/opt/local")
set(ZOLTAN_NO_DEFAULT_PATH "")
if(ZOLTAN_ROOT)
  set(ZOLTAN_SEARCH_PATH "${ZOLTAN_ROOT}")
  set(ZOLTAN_NO_DEFAULT_PATH "NO_DEFAULT_PATH")
endif()

# We only need zoltan with MPI. Otherwise usage of alugrid is broken.
find_package(MPI)

# Make sure we have checked for the underlying partitioners.
find_package(PTScotch)
#find_package(ParMETIS)

# search for files which implements this module
find_path (ZOLTAN_INCLUDE_DIRS
  NAMES "zoltan.h"
  PATHS ${ZOLTAN_SEARCH_PATH}
  PATH_SUFFIXES include trilinos
  ${ZOLTAN_NO_DEFAULT_PATH})

# only search in architecture-relevant directory
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)

find_library(ZOLTAN_LIBRARIES
  NAMES zoltan trilinos_zoltan
  PATHS ${ZOLTAN_SEARCH_PATH}
  PATH_SUFFIXES "lib/.libs" "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  ${ZOLTAN_NO_DEFAULT_PATH})

set (ZOLTAN_FOUND FALSE)

set (ZOLTAN_CONFIG_VAR HAVE_ZOLTAN)

# print a message to indicate status of this package
include (FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZOLTAN
  DEFAULT_MSG
  ZOLTAN_LIBRARIES
  ZOLTAN_INCLUDE_DIRS
  MPI_FOUND
  )

if (ZOLTAN_FOUND)
  set(HAVE_ZOLTAN 1)
  set(ZOLTAN_LIBRARIES ${ZOLTAN_LIBRARIES} ${PARMETIS_LIBRARIES} ${PTSCOTCH_LIBRARIES})
  set(ZOLTAN_INCLUDE_DIRS ${ZOLTAN_INCLUDE_DIRS} ${PARMETIS_INCLUDE_DIRS}
      ${PTSCOTCH_INCLUDE_DIRS})
endif()

