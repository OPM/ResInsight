# - Use precompiled headers
#
# precompile_header takes these parameters
#
#	language      Language in which the header is written; C or CXX.
#
#	type          Type of target being build, SHARED_LIBRARY, STATIC_LIBRARY
#	              or EXECUTABLE.
#
#	header        Relative path within the source tree to the header
#	              that contains the list of includes to be precompiled.
#	              This header should not be added to the installation,
#	              as it will be specific for this project.
#
#	target        Name of target to be created. All targets that
#	              use the precompiled header should depend on this target
#	              so that it is built before them. A variable with this
#	              name will also be created which contains the file name.
#
#	flags_name    Name of variable to receive the flags that should be
#                 added to the command-line.
#
# Example:
#	get_target_property (type opmcore TYPE)
#	precompile_header (CXX ${type}
#		HEADER "opm/core/opm-core-pch.hpp"
#		TARGET opmcore_CXX_pch
#		FLAGS  opmcore_PRECOMP_CXX_FLAGS
#	)
#	set_source_files_properties (${opmcore_CXX_SOURCES} PROPERTIES
#		OBJECT_DEPENDS "${opmcore_CXX_pch}"
#		COMPILE_FLAGS  "${opmcore_PRECOMP_CXX_FLAGS}"
#	)

# get compiler version
include (UseCompVer)
is_compiler_gcc_compatible ()

# reconstruct the compiler command line; this does NOT include the
# DEFINE_SYMBOL that is added for shared libraries. type is the TYPE
# target property.
# see larsch's PrecompiledHeader.cmake: <https://gist.github.com/573926>
# and <https://github.com/loaden/qtcreator/blob/wip/cmake/cmake/PrecompiledHeader.cmake>
function (compiler_cmdline language type cmd_name args_name)
  # get the compiler for this particular language
  set (${cmd_name} "${CMAKE_${language}_COMPILER}" PARENT_SCOPE)
  
  # in case someone has overridden the compiler (e.g. ccache)
  set (_args "${CMAKE_${language}_COMPILER_ARG1}")

  # macro definitions
  get_directory_property (_defs DEFINITIONS)
  list (APPEND _args "${_defs}")

  # global flags (such as -std=c++11); notice that there are both
  # release-dependent and non-release-dependent ones
  string (TOUPPER "CMAKE_${language}_FLAGS" _flags)
  list (APPEND _args "${${_flags}}")
  string (TOUPPER "CMAKE_${language}_FLAGS_${CMAKE_BUILD_TYPE}" _flags)
  list (APPEND _args "${${_flags}}")

  # assume that we are always generating position-independent code
  # when compiling for a shared library
  if (type STREQUAL "SHARED_LIBRARY")
	list (APPEND _args "${CMAKE_SHARED_LIBRARY_${language}_FLAGS}")
  endif (type STREQUAL "SHARED_LIBRARY")

  # directories included
  get_directory_property (_dirs INCLUDE_DIRECTORIES)
  foreach (_dir ${_dirs})
	list (APPEND _args "-I${_dir}")
  endforeach (_dir)

  # make arguments a real list, and write to output variable
  separate_arguments (_args)
  set (${args_name} "${_args}" PARENT_SCOPE)
endfunction (compiler_cmdline language type cmd_name args_name)

function (precompile_header
	language type hdr_kw header tgt_kw target flgs_kw flags_name)
  
  # check "syntax"
  if (NOT hdr_kw STREQUAL "HEADER")
	message (FATAL "Third token to precompile_header shoulde be \"HEADER\"")
  endif (NOT hdr_kw STREQUAL "HEADER")
  if (NOT tgt_kw STREQUAL "TARGET")
	message (FATAL "Fifth token to precompile_header should be \"TARGET\"")
  endif (NOT tgt_kw STREQUAL "TARGET")
  if (NOT flgs_kw STREQUAL "FLAGS")
	message (FATAL "Seventh token to precompile_header should be \"FLAGS\"")
  endif (NOT flgs_kw STREQUAL "FLAGS")

  # check language
  if (language STREQUAL "CXX")
	set (gcc_lang "c++-header")
  elseif (language STREQUAL "C")
	set (gcc_lang "c-header")
  else (language STREQUAL "CXX")
	message (FATAL "Only C or C++ can have precompiled headers")
  endif (language STREQUAL "CXX")

  # if no precompiled header was found, then we shouldn't do anything here
  if (NOT header)
	return ()
  endif (NOT header)
  
  # only support precompiled headers if the compiler is gcc >= 3.4 or clang
  if (CXX_COMPAT_GCC)
	if (CMAKE_COMPILER_IS_GNUCXX)
	  # genuine GCC; must test version
	  get_gcc_version (${language} GCC_VERSION)
	  if (GCC_VERSION VERSION_EQUAL 3.4 OR GCC_VERSION VERSION_GREATER 3.4)
		set (_do_pch TRUE)
	  else ()
		set (_do_pch FALSE)
	  endif ()
	elseif (CMAKE_COMPILER_IS_CLANGXX)
	  # any Clang version that is new enough to compile us can do this
	  set (_do_pch TRUE)
	else ()
	  set (_do_pch FALSE)
	endif ()
	if (_do_pch)
	  # command-line used to compile modules in this kind of target
	  compiler_cmdline (${language} ${type} _cmd _args)

	  # gcc will include any configurations which are in a directory
	  # with the same name as the header included
	  set (_pch_dir "CMakeFiles/pch")
	  set (_pch_file "${_pch_dir}/${header}.gch/${target}")

	  # make sure that output directory exists
	  get_filename_component (_outdir "${PROJECT_BINARY_DIR}/${_pch_file}" PATH)
	  file (MAKE_DIRECTORY ${_outdir})

	  # we need to generate the precompiled header in the output tree, but
	  # at the same time prevent the compiler to pick up the header from the
	  # source tree. getting the order of the include paths right is fragile
	  # in CMake. by copying the header, we can put the precompile dump
	  # right next to it and have the compiler pick it up there
	  add_custom_command (
		OUTPUT "${_pch_dir}/${header}"
		COMMAND ${CMAKE_COMMAND}
		ARGS    -E copy "${PROJECT_SOURCE_DIR}/${header}" "${_pch_dir}/${header}"
		DEPENDS "${PROJECT_SOURCE_DIR}/${header}"
		)

	  # add a makefile rule to create the precompiled header
	  add_custom_command (
		OUTPUT  ${PROJECT_BINARY_DIR}/${_pch_file}
		COMMAND ${_cmd}
		ARGS    ${_args} "-o" "${_pch_file}" "-x" "${gcc_lang}" "-c" "${_pch_dir}/${header}"
		DEPENDS "${_pch_dir}/${header}"
		COMMENT "Precompiling headers ${_pch_file}"
		)

	  # create a phony target that is always built, but which only checks
	  # if the header file is OK (i.e. the header only gets rebuilt if
	  # necessary)
	  add_custom_target (${target} ALL
		DEPENDS ${PROJECT_BINARY_DIR}/${_pch_file}
		)

	  # these flags need to be added to the target
	  set (${target} "${_pch_file}" PARENT_SCOPE)
	  set (${flags_name} "-Winvalid-pch -include ${_pch_dir}/${header}" PARENT_SCOPE)
	endif ()
  endif ()
  
endfunction (precompile_header
  language type header tgt_kw target flgs_kw flags_name)
