# Module that checks whether PT-Scotch is available.
#
# Accepts the following variables:
#
# PTSCOTCH_ROOT:   Prefix where PT-Scotch is installed.
# PTSCOTCH_SUFFIX: Scotch might be compiled using different
#                  integer sizes (int32, int32, long). When
#                  this is is set the headers and libaries
#                  are search under the suffix
#                  include/scotch-${PTSCOTCH_SUFFIX, and
#                  lib/scotch-${PTSCOTCH_SUFFIX}, respectively.
# Sets the following variables:
# PTSCOTCH_INCLUDE_DIRS: All include directories needed to compile PT-Scotch programs.
# PTSCOTCH_LIBRARIES:    Alle libraries needed to link PT-Scotch programs.
# PTSCOTCH_FOUND:        True if PT-Scotch was found.
#
# Provides the following macros:
#
# find_package(PTScotch)

find_package(MPI)
macro(_search_pt_lib libvar libname doc)
  find_library(${libvar} ${libname}
    PATHS ${PTSCOTCH_ROOT} ${PTSCOTCH_ROOT}/lib PATH_SUFFIXES ${PATH_SUFFIXES}
    NO_DEFAULT_PATH
    DOC "${doc}")
  find_library(${libvar} ${libname})
endmacro(_search_pt_lib)

if(PTSCOTCH_SUFFIX)
  set(PATH_SUFFIXES "scotch-${PTSCOTCH_SUFFIX}")
else(PTSCOTCH_SUFFIX)
  set(PATH_SUFFIXES "scotch")
endif(PTSCOTCH_SUFFIX)

include(CMakePushCheckState)
cmake_push_check_state() # Save variables
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MPI_DUNE_INCLUDE_PATH})
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${MPI_DUNE_COMPILE_FLAGS}")

find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h
  PATHS ${PTSCOTCH_ROOT} ${PTSCOTCH_ROOT}/include
  PATH_SUFFIXES ${PATH_SUFFIXES}
  NO_DEFAULT_PATH
  DOC "Include directory of PT-Scotch")
find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h
  PATH_SUFFIXES ${PATH_SUFFIXES})

_search_pt_lib(SCOTCH_LIBRARY scotch "The main Scotch library.")
_search_pt_lib(PTSCOTCH_LIBRARY ptscotch "The main PT-Scotch library.")
_search_pt_lib(PTSCOTCHERR_LIBRARY ptscotcherr "The PT-Scotch error library.")

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "PTScotch"
  DEFAULT_MSG
  PTSCOTCH_INCLUDE_DIR
  SCOTCH_LIBRARY
  PTSCOTCH_LIBRARY
  PTSCOTCHERR_LIBRARY
)
#restore old values
cmake_pop_check_state()

if(PTSCOTCH_FOUND)
  set(PTSCOTCH_INCLUDE_DIRS ${PTSCOTCH_INCLUDE_DIR})
  set(PTSCOTCH_LIBRARIES ${SCOTCH_LIBRARY} ${PTSCOTCH_LIBRARY} ${PTSCOTCHERR_LIBRARY} ${MPI_DUNE_LIBRARIES}
    CACHE FILEPATH "All libraries needed to link programs using PT-Scotch")
  set(PTSCOCH_LINK_FLAGS "${DUNE_MPI_LINK_FLAGS}"
    CACHE STRING "PT-Scotch link flags")
  set(HAVE_PTSCOTCH 1)
  if(NOT TARGET PTScotch::Scotch)
    add_library(PTScotch::Scotch UNKNOWN IMPORTED GLOBAL)
    set_target_properties(PTScotch::Scotch PROPERTIES
      IMPORTED_LOCATION "${SCOTCH_LIBRARY}"
      INCLUDE_DIRECTORIES "${PTSCOTCH_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${PTSCOTCHERR_LIBRARY}")
  endif()
  if(NOT TARGET PTScotch::PTScotch)
    add_library(PTScotch::PTScotch UNKNOWN IMPORTED GLOBAL)
    set_target_properties(PTScotch::PTScotch PROPERTIES
      IMPORTED_LOCATION "${PTSCOTCH_LIBRARY}"
      INCLUDE_DIRECTORIES "${PTSCOTCH_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "PTScotch::Scotch;${MPI_DUNE_LIBRARIES}")
  endif()
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determing location of PT-Scotch succeded:\n"
    "Include directory: ${PTSCOTCH_INCLUDE_DIRS}\n"
    "Library directory: ${PTSCOTCH_LIBRARIES}\n\n")
endif(PTSCOTCH_FOUND)

mark_as_advanced(PTSCOTCH_INCLUDE_DIRS PTSCOTCH_LIBRARIES HAVE_PTSCOTCH)
