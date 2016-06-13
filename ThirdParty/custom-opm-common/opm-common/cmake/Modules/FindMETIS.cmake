# -*-cmake-*-
#
# Try to find the libMETIS graph partioning library
#
# Once done, this will define:
#
#  METIS_FOUND         - system has the libMETIS graph partioning library
#  HAVE_METIS          - like METIS_FOUND, but for the inclusion in config.h
#  METIS_INCLUDE_DIRS  - incude paths to use libMETIS
#  METIS_LIBRARIES     - Link these to use libMETIS

set(METIS_SEARCH_PATH "/usr" "/usr/local" "/opt" "/opt/local")
set(METIS_NO_DEFAULT_PATH "")
if(METIS_ROOT)
  set(METIS_SEARCH_PATH "${METIS_ROOT}")
  set(METIS_NO_DEFAULT_PATH "NO_DEFAULT_PATH")
endif()

# search for files which implements this module
find_path (METIS_INCLUDE_DIRS
  NAMES "metis.h"
  PATHS ${METIS_SEARCH_PATH}
  PATH_SUFFIXES "include" "METISLib" "include/metis"
  ${METIS_NO_DEFAULT_PATH})

# only search in architecture-relevant directory
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)

find_library(METIS_LIBRARIES
  NAMES "metis"
  PATHS ${METIS_SEARCH_PATH}
  PATH_SUFFIXES "lib/.libs" "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  ${METIS_NO_DEFAULT_PATH})

set (METIS_FOUND FALSE)
if (METIS_INCLUDE_DIRS OR METIS_LIBRARIES)
  set(METIS_FOUND TRUE)
  set(HAVE_METIS TRUE)
endif()

# print a message to indicate status of this package
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(METIS
  DEFAULT_MSG
  METIS_LIBRARIES
  METIS_INCLUDE_DIRS
  )
