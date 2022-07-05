# - Compile main library target

option (STRIP_DEBUGGING_SYMBOLS "use separate files for the executable code and the debugging symbols" OFF)

macro (opm_compile opm)
  # some CMake properties do not do list expansion
  string (REPLACE ";" " " ${opm}_LINKER_FLAGS_STR "${${opm}_LINKER_FLAGS}")

  # name of the library should not contain dashes, as CMake will
  # define a symbol with that name, and those cannot contain dashes
  string (REPLACE "-" "" ${opm}_TARGET "${${opm}_NAME}")

  # all public header files are together with the source. prepend our own
  # source path to the one of the dependencies so that our version of any
  # ambigious paths are used.

  option(SILENCE_CROSSMODULE_WARNINGS "Disable warnings from cross-module includes" OFF)
  if (SILENCE_CROSSMODULE_WARNINGS)
     include_directories("${PROJECT_SOURCE_DIR}")
     include_directories (SYSTEM ${${opm}_INCLUDE_DIRS})
     set (${opm}_INCLUDE_DIR "${PROJECT_SOURCE_DIR}")
     set (${opm}_INCLUDE_DIRS ${${opm}_INCLUDE_DIR} ${${opm}_INCLUDE_DIRS})
  else()
     set (${opm}_INCLUDE_DIR "${PROJECT_SOURCE_DIR}")
     set (${opm}_INCLUDE_DIRS ${${opm}_INCLUDE_DIR} ${${opm}_INCLUDE_DIRS})
     include_directories (${${opm}_INCLUDE_DIRS})
  endif()


  # create this library, if there are any compilation units
  link_directories (${${opm}_LIBRARY_DIRS})
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    # Some modules may still export definitions using -D, strip it
    string(REGEX REPLACE "-D" "" _clean_defs "${${opm}_DEFINITIONS}")
    add_compile_definitions(${_clean_defs})
  else()
    add_definitions(${${opm}_DEFINITIONS})
  endif()
  set (${opm}_VERSION "${${opm}_VERSION_MAJOR}.${${opm}_VERSION_MINOR}")
  if (${opm}_SOURCES)
        add_library (${${opm}_TARGET} ${${opm}_LIBRARY_TYPE} ${${opm}_SOURCES})
        set_target_properties (${${opm}_TARGET} PROPERTIES
          SOVERSION ${${opm}_VERSION}
          VERSION ${${opm}_VERSION}
          LINK_FLAGS "${${opm}_LINKER_FLAGS_STR}"
          POSITION_INDEPENDENT_CODE TRUE 
          )
        if (${${opm}_LIBRARY_TYPE} STREQUAL "SHARED")
          # libs that will be linked with the main lib
          string(REGEX REPLACE "([;^])[^;]+\\.a[;$]" "\\1" _public_libs
            "${${opm}_LIBRARIES}")
          # libs that will not actually linked to the library but
          # transitively linked to binaries that link to the main library
          string(REGEX REPLACE "([^;]+\\.[^a][a-zA-Z0-9]*|-[a-z]*)[;$]" "" _interface_libs
            "${${opm}_LIBRARIES}")
        else()
          # Use all libs for real and transitive linking
          set(_public_libs ${${opm}_LIBRARIES})
          unset(_interface)
        endif()
        target_link_libraries (${${opm}_TARGET} PUBLIC ${_public_libs}
          INTERFACE ${_interface_libs})

        if (STRIP_DEBUGGING_SYMBOLS)
	  # queue this executable to be stripped
	  strip_debug_symbols (${${opm}_TARGET} ${opm}_DEBUG)
        endif()
	  add_static_analysis_tests(${opm}_SOURCES ${opm}_INCLUDE_DIRS)
  else (${opm}_SOURCES)
	# unset this variable to signal that no library is generated
	set (${opm}_TARGET)
  endif (${opm}_SOURCES)
  
  # pre-compile common headers; this is setup *after* the library to pick
  # up extra options set there
  if (PRECOMPILE_HEADERS)
	# if we have no library, then use the static setting as this will
	# build the same way as any test programs (no -fPIC option)
	if (${opm}_TARGET)
	  get_target_property (_type ${${opm}_TARGET} TYPE)
	else ()
	  set (_type "STATIC")
	endif ()
	precompile_header (CXX ${_type}
	  HEADER "${${opm}_PRECOMP_CXX_HEADER}"
	  TARGET ${opm}_CXX_pch
	  FLAGS  ${opm}_PRECOMP_CXX_FLAGS
	  )
	# must set property on source files instead of entire target, because
	# it only applies to C++ modules (and cannot be used for C)
	set_source_files_properties (${${opm}_CXX_SOURCES} PROPERTIES
	  OBJECT_DEPENDS "${${opm}_CXX_pch}"
	  COMPILE_FLAGS  "${${opm}_PRECOMP_CXX_FLAGS}"
	  )
	message (STATUS "Precompiled headers: ${${opm}_CXX_pch}")
  endif (PRECOMPILE_HEADERS)

  # we need to know the name of the library which is generated
  if (${opm}_TARGET)
	get_target_property (${opm}_LIBRARY ${${opm}_TARGET} LOCATION)
  endif (${opm}_TARGET)
  
endmacro (opm_compile opm)
