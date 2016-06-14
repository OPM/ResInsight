#
# This module first tests for UG and then sets the necessary flags
# and config.h defines. If UG is found UG_FOUND will be true.
#

# this function is required in order not to pollute the global
# namespace with the macros defined in ug-config*.cmake
function(opmFindUg)
  if(NOT UG_ROOT)
    # check whether UG is in /usr/local
    if(EXISTS "/usr/local/include/ug")
      set(UG_ROOT "/usr/local")

      # check whether UG is in /usr
    elseif(EXISTS "/usr/include/ug")
      set(UG_ROOT "/usr")
      
      # oops
    else()
      message(STATUS "Could not find UG. It seems to be not installed.")
      return()
    endif()
  endif()

  if(UG_ROOT AND NOT UG_DIR)
    # define the directory where the config file resides
    if(EXISTS "${UG_ROOT}/lib/cmake/ug/ug-config.cmake")
      set(UG_DIR ${UG_ROOT}/lib/cmake/ug)
    elseif(EXISTS "${UG_ROOT}/lib64/cmake/ug/ug-config.cmake")
      set(UG_DIR ${UG_ROOT}/lib64/cmake/ug)
    else()
      message(WARNING "Could not find file ug-config.cmake relative to given UG_ROOT")
      return()
    endif()
  endif()

  # include the config mode files kindly provided by UG...
  include(${UG_DIR}/ug-config-version.cmake)
  include(${UG_DIR}/ug-config.cmake)

  set(UG_FOUND "1")
  if(NOT UG_FOR_DUNE STREQUAL "yes")
    set(UG_FOUND "0")
    message(WARNING "UG was not configured for DUNE. Did pass --enable-dune to its configure?")
    return()
  endif()

  set(HAVE_UG ${UG_FOUND})

  # parse version
  string(REGEX REPLACE "([0-9]*)\\.[0-9]*\\..*" "\\1" UG_VERSION_MAJOR "${PACKAGE_VERSION}")
  string(REGEX REPLACE "[0-9]*\\.([0-9]*)\\..*" "\\1" UG_VERSION_MINOR "${PACKAGE_VERSION}")
  string(REGEX REPLACE "[0-9]*\\.[0-9]*\\.([0-9]*).*" "\\1" UG_VERSION_REVISION "${PACKAGE_VERSION}")

  string(REGEX REPLACE ".*-patch([0-9]*)" "\\1" TMP "${PACKAGE_VERSION}")
  if(TMP STREQUAL "${PACKAGE_VERSION}")
    set(UG_VERSION_PATCHLEVEL "")
  else()
    set(UG_VERSION_PATCHLEVEL "${TMP}")
  endif()

  # Adjust compiler/linker arguments
  set(UG_LIBRARY_DIR "${libdir}")
  
  foreach (UG_RAW_LIB "-lugS2" "-lugS3" "-ldevS")
    string(REGEX REPLACE "-l(.*)" "\\1" UG_LIB "${UG_RAW_LIB}")
    set(UG_LIB_FILE  "${UG_LIBRARY_DIR}/lib${UG_LIB}.a")
    if (EXISTS "${UG_LIB_FILE}")
      set(UG_LIBS "${UG_LIBS}" ${UG_LIB_FILE})
    else()
      set(UG_LIBS "${UG_LIBS}" ${UG_LIB})
    endif()
  endforeach()

  set(UG_LIBRARIES "${UG_LIBS}")

  # export all variables which need to be seen globally
  set(UG_FOUND "${UG_FOUND}" PARENT_SCOPE)
  set(HAVE_UG "${HAVE_UG}" PARENT_SCOPE)
  set(UG_INCLUDE_DIRS "${UG_INCLUDES}" PARENT_SCOPE)
  set(UG_LIBRARIES "${UG_LIBRARIES}" PARENT_SCOPE)
  set(UG_VERSION_MAJOR "${UG_VERSION_MAJOR}" PARENT_SCOPE)
  set(UG_VERSION_MINOR "${UG_VERSION_MINOR}" PARENT_SCOPE)
  set(UG_VERSION_REVISION "${UG_VERSION_REVISION}" PARENT_SCOPE)
  set(UG_VERSION_PATCHLEVEL "${UG_VERSION_PATCHLEVEL}" PARENT_SCOPE)

  set(UG_DEFINITIONS "${UG_COMPILE_FLAGS}" PARENT_SCOPE)
endfunction()

if (NOT HAVE_UG)
  opmFindUg()
  
  set(HAVE_UG "${HAVE_UG}" CACHE BOOL "UG library is available")
  set(UG_INCLUDE_DIRS "${UG_INCLUDE_DIRS}" CACHE STRING "Directory containing the headers of the UG library")
  set(UG_LIBRARIES "${UG_LIBRARIES}" CACHE STRING "The libraries which need to be linked to be able to use the UG library")
  set(UG_DEFINITIONS "${UG_DEFINITIONS}" CACHE STRING "The compiler flags for the UG library")
  set(UG_VERSION_MAJOR "${UG_VERSION_MAJOR}" CACHE INT "Major version of the UG release")
  set(UG_VERSION_MINOR "${UG_VERSION_MINOR}" CACHE INT "Minor version of the UG release")
  set(UG_VERSION_REVISION "${UG_VERSION_REVISION}" CACHE INT "Revision of the UG release")
  set(UG_VERSION_PATCHLEVEL "${UG_VERSION_PATCHLEVEL}" CACHE INT "Patchlevel of the UG release")

  mark_as_advanced(HAVE_UG)
  mark_as_advanced(UG_INCLUDE_DIRS)
  mark_as_advanced(UG_LIBRARIES)
  mark_as_advanced(UG_DEFINITIONS)
  mark_as_advanced(UG_VERSION_MAJOR)
  mark_as_advanced(UG_VERSION_MINOR)
  mark_as_advanced(UG_VERSION_REVISION)
  mark_as_advanced(UG_VERSION_PATCHLEVEL)
else()
  set(UG_FOUND "0")
endif()
