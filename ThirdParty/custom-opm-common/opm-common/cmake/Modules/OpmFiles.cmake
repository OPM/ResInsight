# - Identify source code

macro (opm_out_dirs)
  # put libraries in lib/ (no multi-arch support in build tree)
  if(MAIN_SOURCE_FILES)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
  endif()
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
  set (CMAKE_Fortran_MODULE_DIRECTORY "${PROJECT_BINARY_DIR}/CMakeFiles")
endmacro (opm_out_dirs)

# support for some of the variables that are used in Autotools
# template files
macro (opm_auto_dirs)
  set (abs_top_builddir "${PROJECT_BINARY_DIR}")
  set (abs_top_srcdir "${PROJECT_SOURCE_DIR}")
endmacro (opm_auto_dirs)

macro (opm_sources opm)
  # this is necessary to set so that we know where we are going to
  # execute the test programs (and make datafiles available)
  set (tests_DIR "tests")

  # how to retrieve the "fancy" name from the filename
  set (tests_REGEXP
	"^test_([^/]*)$"
	"^([^/]*)_test$"
	)

  # these are the lists that must be defined in CMakeLists_files
  # - MAIN_SOURCE_FILES
  # - EXAMPLE_SOURCE_FILES
  # - TEST_SOURCE_FILES
  # - TEST_DATA_FILES
  # - PUBLIC_HEADER_FILES
  # - PROGRAM_SOURCE_FILES
  # - ADDITIONAL_SOURCE_FILES

  # rename from "friendly" names to ones that fit the "almost-structural"
  # scheme used in the .cmake modules, converting them to absolute file
  # names in the process
  foreach (_file IN LISTS MAIN_SOURCE_FILES)
	list (APPEND ${opm}_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
	# further classify into language if some other modules need to add props
	if (_file MATCHES ".*\\.[cC][a-zA-Z]*$")
	  if (_file MATCHES ".*\\.c$")
		list (APPEND ${opm}_C_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
	  else (_file MATCHES ".*\\.c$")
		list (APPEND ${opm}_CXX_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
	  endif (_file MATCHES ".*\\.c$")
	elseif (_file MATCHES ".*\\.[fF][a-zA-Z]*$")
	  list (APPEND ${opm}_Fortran_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
	endif (_file MATCHES ".*\\.[cC][a-zA-Z]*$")
  endforeach (_file)
  foreach (_file IN LISTS PUBLIC_HEADER_FILES)
	list (APPEND ${opm}_HEADERS ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)
  foreach (_file IN LISTS TEST_SOURCE_FILES)
	list (APPEND tests_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)
  foreach (_file IN LISTS TEST_DATA_FILES)
	list (APPEND tests_DATA ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)
  foreach (_file IN LISTS EXAMPLE_SOURCE_FILES)
	list (APPEND examples_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)
  foreach (_file IN LISTS ADDITIONAL_SOURCE_FILES)
	list (APPEND additionals_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)
  foreach (_file IN LISTS PROGRAM_SOURCE_FILES)
	list (APPEND examples_SOURCES_DIST ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)
  foreach (_file IN LISTS ATTIC_FILES)
	list (APPEND attic_SOURCES ${PROJECT_SOURCE_DIR}/${_file})
  endforeach (_file)

  # identify pre-compile header; if the project is called opm-foobar,
  # then it should be in opm/foobar/opm-foobar-pch.hpp
  string (REPLACE "-" "/" opm_NAME_AS_DIR ${${opm}_NAME})
  set (${opm}_PRECOMP_CXX_HEADER "${opm_NAME_AS_DIR}/${${opm}_NAME}-pch.hpp")
  if (NOT EXISTS ${PROJECT_SOURCE_DIR}/${${opm}_PRECOMP_CXX_HEADER})
	set (${opm}_PRECOMP_CXX_HEADER "")
  endif (NOT EXISTS ${PROJECT_SOURCE_DIR}/${${opm}_PRECOMP_CXX_HEADER})
endmacro (opm_sources opm)

# disable an entire directory from sources
macro (opm_disable_source opm)
  foreach (_exp IN ITEMS ${ARGN})
	# regexp or directory?
	if (IS_ABSOLUTE "${_exp}")
	  set (_prefix "")
	else (IS_ABSOLUTE "${_exp}")
	  set (_prefix "${PROJECT_SOURCE_DIR}/")
	endif (IS_ABSOLUTE "${_exp}")
	if (IS_DIRECTORY "${_prefix}${_exp}")
	  set (_glob "/*")
	else (IS_DIRECTORY "${_prefix}${_exp}")
	  set (_glob "")
	endif (IS_DIRECTORY "${_prefix}${_exp}")
	file (GLOB_RECURSE _disabled RELATIVE ${PROJECT_SOURCE_DIR} "${_prefix}${_exp}${_glob}")
	foreach (_file IN ITEMS ${_disabled})
	  list (REMOVE_ITEM ${opm}_SOURCES "${PROJECT_SOURCE_DIR}/${_file}")
	endforeach (_file)
  endforeach (_exp)
endmacro (opm_disable_source opm reldir)
