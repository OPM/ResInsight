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
  set (${opm}_INCLUDE_DIR "${PROJECT_SOURCE_DIR}")
  set (${opm}_INCLUDE_DIRS ${${opm}_INCLUDE_DIR} ${${opm}_INCLUDE_DIRS})

  # create this library, if there are any compilation units
  include_directories (${${opm}_INCLUDE_DIRS})
  link_directories (${${opm}_LIBRARY_DIRS})
  add_definitions (${${opm}_DEFINITIONS})
  set (${opm}_VERSION "${${opm}_VERSION_MAJOR}.${${opm}_VERSION_MINOR}")
  if (${opm}_SOURCES)
	add_library (${${opm}_TARGET} ${${opm}_LIBRARY_TYPE} ${${opm}_SOURCES})
	set_target_properties (${${opm}_TARGET} PROPERTIES
	  SOVERSION ${${opm}_VERSION_MAJOR}
	  VERSION ${${opm}_VERSION}
	  LINK_FLAGS "${${opm}_LINKER_FLAGS_STR}"
          POSITION_INDEPENDENT_CODE TRUE 
          )
	target_link_libraries (${${opm}_TARGET} ${${opm}_LIBRARIES})

        if (STRIP_DEBUGGING_SYMBOLS)
	  # queue this executable to be stripped
	  strip_debug_symbols (${${opm}_TARGET} ${opm}_DEBUG)
        endif()
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
