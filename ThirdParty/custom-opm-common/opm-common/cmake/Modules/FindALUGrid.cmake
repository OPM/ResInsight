#find_package(PkgConfig)

include(CheckLibraryExists) 

macro(_opm_set_alugrid val)
  set(ALUGRID_FOUND ${val})

  if(NOT ALUGRID_FOUND AND ALUGRID_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find required libary ALUGrid")
  endif()

  # print status message if requested
  if(NOT ALUGRID_FIND_QUIETLY)
    if(ALUGRID_FOUND)
      message(STATUS "Found ALUGrid")
    else()
      message(STATUS "Could not find ALUGrid")
    endif()
  endif()
endmacro()

if(ALUGRID_ROOT)
  find_path(ALUGRID_PKGCONFIG_DIR alugrid.pc PATHS ${ALUGRID_ROOT}
    PATH_SUFFIXES lib/pkgconfig/ alugrid/lib/pkgconfig
    NO_DEFAULT_PATH)

  find_file(ALUGRID_VERSION alugridversion PATHS ${ALUGRID_ROOT}/bin
    NO_DEFAULT_PATH)
else()
  find_path(ALUGRID_PKGCONFIG_DIR alugrid.pc
    PATH_SUFFIXES lib/pkgconfig/ alugrid/lib/pkgconfig)

  get_filename_component(_GUESSED_ALUGRID_ROOT ${ALUGRID_PKGCONFIG_DIR}/../../ ABSOLUTE)
  find_file(ALUGRID_VERSION alugridversion PATHS ${_GUESSED_ALUGRID_ROOT}/bin NO_DEFAULT_PATH)

  if(ALUGRID_VERSION)
    set(ALUGRID_ROOT ${_GUESSED_ALUGRID_ROOT})
  else(ALUGRID_VERSION_PATH)
    get_filename_component(_GUESSED_ALUGRID_ROOT ${ALUGRID_PKGCONFIG_DIR}/../../.. ABSOLUTE)
    find_file(ALUGRID_VERSION alugridversion
      PATHS ${_GUESSED_ALUGRID_ROOT}
      PATH_SUFFIXES bin
      NO_DEFAULT_PATH)
    if(ALUGRID_VERSION)
      set(ALUGRID_ROOT ${_GUESSED_ALUGRID_ROOT})
    endif(ALUGRID_VERSION)
  endif(ALUGRID_VERSION)
endif()
unset(ALUGRID_PKGCONFIG_DIR CACHE)

set(ALUGRID_VERSION_REQUIRED 1.50)

if(NOT ALUGRID_VERSION)
  message(STATUS "Could not find ALUGrid.")
  _opm_set_alugrid(0)
  return()
endif(NOT ALUGRID_VERSION)

execute_process(COMMAND ${ALUGRID_VERSION} -c ${ALUGRID_VERSION_REQUIRED} OUTPUT_VARIABLE ALUGRID_VERSION)
if(ALUGRID_VERSION LESS 0)
  message(STATUS "ALUGrid version is less than ${ALUGRID_VERSION_REQUIRED}")
  _opm_set_alugrid(0)
  unset(ALUGRID_VERSION CACHE)
  return()
else()
  message(STATUS "ALUGrid version is compatible")
endif()
unset(ALUGRID_VERSION CACHE)

find_path(ALUGRID_INCLUDE_DIR "alugrid_serial.h"
  PATHS "${ALUGRID_ROOT}" PATH_SUFFIXES "include" "include/serial"
  NO_DEFAULT_PATH DOC "Include path of serial alugrid headers.")
if (NOT ALUGRID_INCLUDE_DIR)
  message(STATUS "Could not deterimine ALUGrid include directory")
  _opm_set_alugrid(0)
  return()  
endif()
mark_as_advanced(ALUGRID_INCLUDE_DIR)

find_library(ALUGRID_LIBRARY alugrid
  PATHS "${ALUGRID_ROOT}" 
  PATH_SUFFIXES lib lib32 lib64 
  DOC "ALUGrid library"
  NO_DEFAULT_PATH)
if (NOT ALUGRID_LIBRARY)
  message(STATUS "Could not find ALUGrid usable library")
  _opm_set_alugrid(0)
  return()  
endif()
set(ALUGRID_LIBRARIES ${ALUGRID_LIBRARY})
mark_as_advanced(ALUGRID_LIBRARIES)

set(ALUGRID_INCLUDE_DIRS
  ${ALUGRID_INCLUDE_DIR}
  ${ALUGRID_INCLUDE_DIR}/serial
  ${ALUGRID_INCLUDE_DIR}/duneinterface)

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ALUGRID_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${ALUGRID_LIBRARIES})
check_include_file_cxx(stlheaders.h ALUGRID_SERIAL_FOUND)

if(ALUGRID_SERIAL_FOUND)
  check_cxx_source_compiles("#include <alugrid_defineparallel.h> 
                              #if ALU3DGRID_BUILD_FOR_PARALLEL == 0 
                              #error
                              #endif
                              int main(){}"
                              ALUGRID_PARALLEL_FOUND)
else()
  message(STATUS "alugrid_serial.h found, but could not be compiled.")
  _opm_set_alugrid(0)
  return() 
endif()

if(ALUGRID_PARALLEL_FOUND AND MPI_FOUND)
  # must link with METIS if we are going to use parallel ALUGrid
  find_package(METIS)
  if (METIS_FOUND)
	list(APPEND ALUGRID_LIBRARIES ${METIS_LIBRARIES})

	# check for parallel ALUGrid
	find_path(ALUGRID_PARALLEL_INCLUDE_PATH "alumetis.hh"
      PATHS ${ALUGRID_INCLUDE_DIR} 
      PATH_SUFFIXES "parallel"
      NO_DEFAULT_PATH)

	if(ALUGRID_PARALLEL_INCLUDE_PATH)
      list(APPEND ALUGRID_INCLUDE_DIRS ${ALUGRID_PARALLEL_INCLUDE_PATH})
      set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ALUGRID_INCLUDE_DIRS})
      set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${ALUGRID_LIBRARIES})
      #set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -DSOME_MORE_DEF)
      check_include_file_cxx(alugrid_parallel.h ALUGRID_PARALLEL_FOUND)
      unset(ALUGRID_PARALLEL_INCLUDE_PATH CACHE)
    
      if(NOT ALUGRID_PARALLEL_FOUND)
		message(STATUS "alumetis.hh not found  in ${ALUGRID_PARALLEL_INCLUDE_PATH}")
      endif()
	else()
	  message (STATUS "METIS not found, parallel ALUGrid disabled")
	  set (ALUGRID_PARALLEL_FOUND 0)
	endif()
  else()
    message(STATUS "alumetis.hh not found (required by parallel alugrid).")
    set(ALUGRID_PARALLEL_FOUND 0)
  endif()
endif()

if(ALUGRID_SERIAL_FOUND)
  _opm_set_alugrid(1)
endif(ALUGRID_SERIAL_FOUND)
