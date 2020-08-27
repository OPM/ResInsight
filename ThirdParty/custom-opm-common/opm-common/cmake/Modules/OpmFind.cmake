# - Generic inclusion of packages
#
# Synopsis:
#
#	find_and_append_package (name args)
#
# where
#
#	name          Name of the package, e.g. Boost
#   args          Other arguments, e.g. COMPONENTS, REQUIRED, QUIET etc.
#
# This macro will append the list of standard variables found by the
# package to this project's standard variables
#
########################################################################
#
# - Generic inclusion of a list of packages
#
# Synopsis:
#
#	find_and_append_package_list (args)
#
# where
#
#	args          List of package strings. Each string must be quoted if
#	              it contains more than one word.
#
# Example:
#
#	find_and_append_package_list (
#		"Boost COMPONENTS filesystem REQUIRED"
#		SUPERLU
#	)

include (Duplicates)
include (OpmSiblingSearch)

# list of suffixes for all the project variables
set (_opm_proj_vars
  SOURCES
  LINKER_FLAGS
  LIBRARIES
  DEFINITIONS
  INCLUDE_DIRS
  LIBRARY_DIRS
  CONFIG_VARS
  CONFIG_IMPL_VARS
  )

# ensure that they are at least the empty list after we're done
foreach (name IN LISTS _opm_proj_vars)
  if (NOT DEFINED ${CMAKE_PROJECT_NAME}_${name})
	set (${CMAKE_PROJECT_NAME}_${name} "")
  endif (NOT DEFINED ${CMAKE_PROJECT_NAME}_${name})
endforeach (name)


# insert this boilerplate whenever we are going to find a new package
macro (find_and_append_package_to prefix name)
  # special handling for Boost to avoid inadvertedly picking up system
  # libraries when we want our own version. this is done here because
  # having a custom Boost is common, but the logic to search only there
  # does not follow any particular convention.
  if (BOOST_ROOT AND NOT DEFINED Boost_NO_SYSTEM_PATHS)
	set (Boost_NO_SYSTEM_PATHS TRUE)
  endif (BOOST_ROOT AND NOT DEFINED Boost_NO_SYSTEM_PATHS)

  # if we have specified a directory, don't revert to searching the
  # system default paths afterwards
  string (TOUPPER "${name}" NAME)
  string (REPLACE "-" "_" NAME "${NAME}")

  # only use suite if module-specific variable is not set. this allows
  # us to override one dir in a suite
  if (NOT (${name}_DIR OR ${name}_ROOT OR ${NAME}_ROOT))
	# module is part of a suite if it has name with the pattern xxx-yyy
	if (("${name}" MATCHES "[^-]+-.+") OR ${name}_SUITE)
	  # allow to override if the module doesn't quite fit the convention
	  # e.g. dune-cornerpoint (since renamed to opm-grid)
	  if (NOT DEFINED ${name}_SUITE)
		# extract suite name from module
		string (REGEX REPLACE "([^-]+)-.+" "\\1" ${name}_SUITE "${name}")
	  endif (NOT DEFINED ${name}_SUITE)
	  # assume that each module has its own subdir directly under suite dir
	  string (TOUPPER "${${name}_SUITE}" ${name}_SUITE_UPPER)
	  if (DEFINED ${${name}_SUITE_UPPER}_ROOT)
		set (${NAME}_ROOT ${${${name}_SUITE_UPPER}_ROOT}/${name})
	  endif (DEFINED ${${name}_SUITE_UPPER}_ROOT)
	endif (("${name}" MATCHES "[^-]+-.+") OR ${name}_SUITE)
  endif (NOT (${name}_DIR OR ${name}_ROOT OR ${NAME}_ROOT))

  # the documentation says that if *-config.cmake files are not found,
  # find_package will revert to doing a full search, but that is not
  # true, so unconditionally setting ${name}_DIR is not safe. however,
  # if the directory given to us contains a config file, then copy the
  # value over to this variable to switch to config mode (CMake will
  # always use config mode if *_DIR is defined)
  if (NOT DEFINED ${name}_DIR AND (DEFINED ${name}_ROOT OR DEFINED ${NAME}_ROOT))
	if (EXISTS ${${name}_ROOT}/${name}-config.cmake OR EXISTS ${${name}_ROOT}/${name}Config.cmake)
	  set (${name}_DIR "${${name}_ROOT}")
	endif (EXISTS ${${name}_ROOT}/${name}-config.cmake OR EXISTS ${${name}_ROOT}/${name}Config.cmake)
	if (EXISTS ${${NAME}_ROOT}/${name}-config.cmake OR EXISTS ${${NAME}_ROOT}/${name}Config.cmake)
	  set (${name}_DIR "${${NAME}_ROOT}")
	endif (EXISTS ${${NAME}_ROOT}/${name}-config.cmake OR EXISTS ${${NAME}_ROOT}/${name}Config.cmake)
  endif (NOT DEFINED ${name}_DIR AND (DEFINED ${name}_ROOT OR DEFINED ${NAME}_ROOT))

  # if we're told not to look for the package, pretend it was never found
  if (CMAKE_DISABLE_FIND_PACKAGE_${name})
    # If required send an error
    cmake_parse_arguments(FIND "REQUIRED" "" "" ${ARGN} )
    set (${name}_FOUND FALSE)
    set (${NAME}_FOUND FALSE)
    if (FIND_REQUIRED)
        message(SEND_ERROR "package ${name} but disable with CMAKE_DISABLE_FIND_PACKAGE_${name}")
    endif ()
  else ()
    # List of components might differ for every module. Therefore we will
    # need to research for a library multiple times. _search_components
    # will hold the index of the string COMPONENTS in the list
    set(_ARGN_LIST ${ARGN}) # Create a real list to use with list commands
    list(FIND _ARGN_LIST "COMPONENTS" _search_components)

    # using config mode is better than using module (aka. find) mode
    # because then the package has already done all its probes and
    # stored them in the config file for us
    # For dune and opm modules and exempted packages we force module mode.
    # For dune and opm it will use config mode underneath.
    # We even need to repeat the search for opm-common once as this is done
    # in the top most CMakeLists.txt without querying defines, setting dependencies
    # and the likes which is only done via opm_find_package
    if ( (NOT DEFINED ${name}_FOUND AND NOT DEFINED ${NAME}_FOUND )
         OR _search_components GREATER -1)
       string(REGEX MATCH "(opm)-.*" _is_opm ${name})
      if(NOT _is_opm)
        find_package (${name} ${ARGN})
      else()
        if(${name}_DIR)
          find_package (${name} ${${prefix}_VERSION_MAJOR}.${${prefix}_VERSION_MINOR} ${ARGN} NO_MODULE PATHS ${${name}_DIR} NO_DEFAULT_PATH)
        else()
          find_package (${name} ${${prefix}_VERSION_MAJOR}.${${prefix}_VERSION_MINOR} ${ARGN} NO_MODULE)
        endif()
        include(FindPackageHandleStandardArgs)
        if(${name}_FOUND AND ${name}_LIBRARY STREQUAL "")
          find_package_handle_standard_args(${name}
                                            REQUIRED_VARS ${name}_INCLUDE_DIRS)
        else()
          find_package_handle_standard_args(${name}
                                            REQUIRED_VARS ${name}_LIBRARY)
        endif()
      endif ()
    endif ()
    if (NOT DEFINED ${name}_FOUND)
      set (${name}_FOUND "${${NAME}_FOUND}")
    endif ()
    if (NOT DEFINED ${NAME}_FOUND)
      set (${NAME}_FOUND "${${name}_FOUND}")
    endif ()
  endif ()

  # the variable "NAME" may be replaced during find_package (as this is
  # now a macro, and not a function anymore), so we must reinitialize
  string (TOUPPER "${name}" NAME)
  string (REPLACE "-" "_" NAME "${NAME}")

  if (${name}_FOUND OR ${NAME}_FOUND)
	foreach (var IN LISTS _opm_proj_vars)
	  if (DEFINED ${name}_${var})
		list (APPEND ${prefix}_${var} ${${name}_${var}})
	  # some packages define an uppercase version of their own name
	  elseif (DEFINED ${NAME}_${var})
		list (APPEND ${prefix}_${var} ${${NAME}_${var}})
	  endif (DEFINED ${name}_${var})
	  # some packages define _PATH instead of _DIRS (Hi, MPI!)
	  if ("${var}" STREQUAL "INCLUDE_DIRS")
		if (DEFINED ${name}_INCLUDE_PATH)
		  list (APPEND ${prefix}_INCLUDE_DIRS ${${name}_INCLUDE_PATH})
		elseif (DEFINED ${NAME}_INCLUDE_PATH)
		  list (APPEND ${prefix}_INCLUDE_DIRS ${${NAME}_INCLUDE_PATH})
		endif (DEFINED ${name}_INCLUDE_PATH)
		# some packages define only _DIR and not _DIRS (Hi, Eigen3!)
		if (DEFINED ${name}_INCLUDE_DIR)
		  list (APPEND ${prefix}_INCLUDE_DIRS ${${name}_INCLUDE_DIR})
		elseif (DEFINED ${NAME}_INCLUDE_DIR)
		  list (APPEND ${prefix}_INCLUDE_DIRS ${${NAME}_INCLUDE_DIR})
		endif (DEFINED ${name}_INCLUDE_DIR)
	  endif ("${var}" STREQUAL "INCLUDE_DIRS")
	  # cleanup lists
	  if ("${var}" STREQUAL "LIBRARIES")
		remove_duplicate_libraries (${prefix})
	  else ("${var}" STREQUAL "LIBRARIES")
		remove_duplicate_var (${prefix} ${var})
	  endif ("${var}" STREQUAL "LIBRARIES")
	endforeach (var)
	# some libraries only define xxx_FOUND and not a corresponding HAVE_xxx
	if (NOT DEFINED HAVE_${NAME})
	  set (HAVE_${NAME} 1)
	endif (NOT DEFINED HAVE_${NAME})
  endif (${name}_FOUND OR ${NAME}_FOUND)
endmacro (find_and_append_package_to prefix name)

# append to the list of variables associated with the project
macro (find_and_append_package name)
  find_and_append_package_to (${CMAKE_PROJECT_NAME} ${name} ${ARGN})
endmacro (find_and_append_package name)

# find a list of dependencies, adding each one of them
macro (find_and_append_package_list_to prefix)
  # setting and separating is necessary to work around apparent bugs
  # in CMake's parser (sic)
  set (_deps ${ARGN})
  foreach (_dep IN LISTS _deps)
	separate_arguments (_args UNIX_COMMAND ${_dep})
	find_and_append_package_to (${prefix} ${_args})
  endforeach (_dep)
endmacro (find_and_append_package_list_to prefix)

# convenience method to supply the project name as prefix
macro (find_and_append_package_list)
  find_and_append_package_list_to (${CMAKE_PROJECT_NAME} ${ARGN})
endmacro (find_and_append_package_list)
