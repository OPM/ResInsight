# Module that checks whether ParMETIS or the ParMETIS interface of PT-Scotch
# is available.
#
# Accepts the following variables:
#
# PARMETIS_ROOT:   Prefix where ParMETIS is installed.
# PARMETIS_SUFFIX: Scotch might be compiled using different
#                  integer sizes (int32, int32, long). When
#                  this is is set the headers and libaries
#                  are search under the suffix
#                  include/parmetis-${PARMETIS_SUFFIX}, and
#                  lib/parmetis-${PARMETIS_SUFFIX}, respectively.
# Sets the following variables:
# PARMETIS_INCLUDE_DIRS: All include directories needed to compile ParMETIS programs.
# PARMETIS_LIBRARIES:    Alle libraries needed to link ParMETIS programs.
# PARMETIS_FOUND:        True if ParMETIS was found.
#
# Provides the following macros:
#
# find_package(ParMETIS)

find_package(MPI)

if(MPI_C_FOUND)
macro(_search_parmetis_lib libvar libname doc)
  find_library(${libvar} ${libname}
    PATHS ${PARMETIS_ROOT} ${PARMETIS_ROOT}/lib PATH_SUFFIXES ${PATH_SUFFIXES}
    NO_DEFAULT_PATH
    DOC "${doc}")
  find_library(${libvar} ${libname})
endmacro(_search_parmetis_lib)

if(PARMETIS_SUFFIX)
  set(PATH_SUFFIXES "-${PARMETIS_SUFFIX}")
else(PARMETIS_SUFFIX)
  set(PATH_SUFFIXES "")
endif(PARMETIS_SUFFIX)

include(CMakePushCheckState)
cmake_push_check_state() # Save variables

find_path(PARMETIS_INCLUDE_DIR parmetis.h
  PATHS ${PARMETIS_ROOT}   ${PARMETIS_ROOT}/include
  PATH_SUFFIXES parmetis${PATH_SUFFIXES}
  NO_DEFAULT_PATH
  DOC "Include directory of ParMETIS")
find_path(PARMETIS_INCLUDE_DIR parmetis.h
  PATH_SUFFIXES parmetis${PATH_SUFFIXES})

# find the serial version of METIS
find_package(METIS)
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MPI_C_INCLUDE_PATH} )
if(PARMETIS_INCLUDE_DIR)
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${PARMETIS_INCLUDE_DIR})
  if(METIS_INCLUDE_DIRS)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${METIS_INCLUDE_DIRS})
  endif()
endif()
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${MPI_C_COMPILE_FLAGS}")

include(CheckIncludeFile)
check_include_file(parmetis.h PARMETIS_FOUND)

if(NOT PARMETIS_FOUND)
  # If we are using the ParMETIS bindings of PTScotch, we need
  # to use the scotch include path as partmetis.h includes scotch.h
  find_package(PTScotch)
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${PTSCOTCH_INCLUDE_DIR})
    unset(PARMETIS_FOUND CACHE) # force recheck of include file
    check_include_file(parmetis.h PARMETIS_FOUND)
    if(PARMETIS_FOUND)
      set(PARMETIS_SCOTCH_INCLUDE_DIRS ${PTSCOTCH_INCLUDE_DIRS})
    endif()
endif()

_search_parmetis_lib(PARMETIS_LIBRARY parmetis "The main ParMETIS library.")

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "ParMETIS"
  DEFAULT_MSG
  PARMETIS_INCLUDE_DIR
  PARMETIS_LIBRARY
  METIS_LIBRARIES
  PARMETIS_FOUND
  METIS_FOUND
)
#restore old values
cmake_pop_check_state()

if(PARMETIS_FOUND)
  set(PARMETIS_INCLUDE_DIRS ${PARMETIS_INCLUDE_DIR} ${PARMETIS_SCOTCH_INCLUDE_DIRS})
  set(PARMETIS_LIBRARIES ${PARMETIS_LIBRARY} ${METIS_LIBRARIES} ${MPI_C_LIBRARIES}
    CACHE FILEPATH "All libraries needed to link programs using ParMETIS")
  set(PARMETIS_LINK_FLAGS "${DUNE_C_LINK_FLAGS}"
    CACHE STRING "ParMETIS link flags")
  set(HAVE_PARMETIS 1)
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determining location of ParMETIS succeded:\n"
    "Include directory: ${PARMETIS_INCLUDE_DIRS}\n"
    "Library directory: ${PARMETIS_LIBRARIES}\n\n")

  if(NOT TARGET ParMETIS::ParMETIS)
    add_library(ParMETIS::ParMETIS UNKNOWN IMPORTED GLOBAL)
    set_target_properties(ParMETIS::ParMETIS PROPERTIES
      IMPORTED_LOCATION ${PARMETIS_LIBRARY}
      INCLUDE_DIRECTORIES "${PARMETIS_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${METIS_LIBRARIES};${MPI_C_LIBRARIES}")
  endif()
endif(PARMETIS_FOUND)

mark_as_advanced(PARMETIS_INCLUDE_DIRS PARMETIS_LIBRARIES HAVE_PARMETIS)
else(MPI_C_FOUND)
  message(WARNING "MPI not found ==> ParMETIS disabled! Plase make sure -DUSE_MPI=ON was set if you need ParMETIS.")
endif(MPI_C_FOUND)
