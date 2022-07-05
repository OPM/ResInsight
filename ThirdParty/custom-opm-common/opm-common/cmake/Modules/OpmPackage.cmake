# - Find routine for OPM-like modules
#
# Synopsis:
#
#   find_opm_package (module deps header lib defs prog conf)
#
# where
#
#   module    Name of the module, e.g. "dune-common"; this will be the
#             stem of all variables defined (see below).
#   deps      Semi-colon-separated list of dependent modules which must
#             be present; those that are required must be marked as such
#	          explicitly. Quote if more than one word is necessary to
#	          describe the dependency.
#   header    Name of the header file to probe for, e.g.
#             "dune/common/fvector.hh". Note that you should have to same
#             relative path here as is used in the header files.
#   lib       Name of the library to probe for, e.g. "dunecommon"
#   defs      Symbols that should be passed to compilations
#   prog      Program that should compile if library is present
#   conf      Symbols that should be present in config.h
#
# It will provide these standard Find-module variables:
#
#   ${module}_INCLUDE_DIRS    Directory of header files
#   ${module}_LIBRARIES       Directory of shared object files
#   ${module}_DEFINITIONS     Defines that must be set to compile
#   ${module}_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_${MODULE}            Binary value to use in config.h
#
# Note: Arguments should be quoted, otherwise a list will spill into the
#       next argument!

# Copyright (C) 2012 Uni Research AS
# This file is licensed under the GNU General Public License v3.0

# <http://www.vtk.org/Wiki/CMake:How_To_Find_Libraries>

include (OpmFind)
include (OpmSiblingSearch)

# append all items from src into dst; both must be *names* of lists
macro (append_found src dst)
  foreach (_item IN LISTS ${src})
	if (NOT "${_item}" MATCHES "-NOTFOUND$")
	  list (APPEND ${dst} ${_item})
	endif (NOT "${_item}" MATCHES "-NOTFOUND$")
  endforeach (_item)
endmacro (append_found src dst)

macro (find_opm_package module deps header lib defs prog conf)
  # in addition to accepting mod-ule_ROOT, we also accept the somewhat
  # more idiomatic MOD_ULE_ROOT variant
  string (TOUPPER "${module}" MODULE_UPPER)
  string (REPLACE "-" "_" MODULE "${MODULE_UPPER}")

  # if someone else has included this test, don't do it again
  # one exception is opm-common which is already found in the
  # top most CMakeLists.txt but we still need to search for its
  # dependencies
  if (${MODULE}_FOUND OR ${module}_FOUND)
      return ()
  endif ()

  # variables to pass on to other packages
  if (${module}_FIND_QUIETLY)
	set (_${module}_quiet "QUIET")
  else (${module}_FIND_QUIETLY)
	set (_${module}_quiet "")
  endif (${module}_FIND_QUIETLY)
  if (${module}_FIND_REQUIRED)
	set (_${module}_required "REQUIRED")
  else (${module}_FIND_REQUIRED)
	set (_${module}_required "")
  endif (${module}_FIND_REQUIRED)

  create_module_dir_var(${module})

  # This will also set all the needed variables with the exception of
  # ${module}_CONFIG_VARS for dune modules.
  find_package(${module} ${_${module}_quiet} ${_${module}_required} CONFIG)

  if(NOT ${module}_DEPS)
    # set the dependencies used in find_package_deps
    set(${module}_DEPS "${deps}")
  endif()
  find_package_deps(${module})

  # since find_and_append_package_to is a macro, this variable have
  # probably been overwritten (due to its common name); it is now
  # this module's last dependency instead of the name of the module
  # itself, so it must be restored
  string (TOUPPER "${module}" MODULE_UPPER)
  string (REPLACE "-" "_" MODULE "${MODULE_UPPER}")

  # compile with this option to avoid avalanche of warnings
  set (${module}_DEFINITIONS "${${module}_DEFINITIONS}")
  # -D to compile definitions for older CMake versions
  set (_D_PREFIX "")
  if(CMAKE_VERSION VERSION_LESS "3.12")
    set(_D_PREFIX "-D")
  endif()
  foreach (_def IN ITEMS ${defs})
    if(_def MATCHES "^[A-Za-z].*")
      list (APPEND ${module}_DEFINITIONS "${_D_PREFIX}${_def}")
    endif()
  endforeach (_def)

  list (APPEND ${module}_DEFINITIONS ${defs})

  # tidy the lists before returning them
  remove_dup_deps (${module})

  # these defines are used in dune/${module} headers, and should be put
  # in config.h when we include those
  foreach (_var IN ITEMS ${conf})
    # massage the name to remove source code formatting
    string (REGEX REPLACE "^[\n\t\ ]+" "" _var "${_var}")
    string (REGEX REPLACE "[\n\t\ ]+$" "" _var "${_var}")
    list (APPEND ${module}_CONFIG_VARS ${_var})
  endforeach (_var)

  # these are the defines that should be set when compiling
  # without config.h
  config_cmd_line (${module}_CMD_CONFIG ${module}_CONFIG_VARS)

  if(prog)
    # check that we can compile a small test-program
    include (CMakePushCheckState)
    cmake_push_check_state ()
    include (CheckCXXSourceCompiles)
    # only add these if they are actually found; otherwise it won't
    # compile and the variable won't be set
    append_found (${module}_INCLUDE_DIRS CMAKE_REQUIRED_INCLUDES)
    append_found (${module}_LIBRARIES CMAKE_REQUIRED_LIBRARIES)
    # since we don't have any config.h yet
    list (APPEND CMAKE_REQUIRED_DEFINITIONS ${${module}_DEFINITIONS})
    list (APPEND CMAKE_REQUIRED_DEFINITIONS ${${module}_CMD_CONFIG})
    check_cxx_source_compiles ("${prog}" HAVE_${MODULE})
    cmake_pop_check_state ()
  else(prog)
    if(${module}_FOUND)
      # No test code provided, mark compilation as successful
      # if module was founf
      set(HAVE_${MODULE} 1)
    endif(${module}_FOUND)
  endif(prog)

  # write status message in the same manner as everyone else
  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args (
        ${module}
        DEFAULT_MSG
        ${module}_INCLUDE_DIRS ${module}_LIBRARIES ${module}_FOUND ${module}_ALL_PREREQS HAVE_${MODULE}
        )

  # some genius that coded the FindPackageHandleStandardArgs figured out
  # that the module name should be in uppercase (?!)
  set (${module}_FOUND "${${MODULE_UPPER}_FOUND}")
  set (${MODULE}_FOUND "${${MODULE_UPPER}_FOUND}")

  # This variable is used by UseDuneVer
  list(GET ${module}_INCLUDE_DIRS 0 ${module}_INCLUDE_DIR)
  # print everything out if we're asked to
  if (${module}_DEBUG)
	debug_find_vars (${module})
  endif (${module}_DEBUG)
endmacro (find_opm_package module deps header lib defs prog conf)

macro (find_package_deps module)
  # period because it should be something that evaluates to true
  # in find_package_handle_standard_args
  set (${module}_ALL_PREREQS ".")
  foreach (_dep IN ITEMS ${${module}_DEPS})
	separate_arguments (_${module}_args UNIX_COMMAND "${_dep}")
	if (_${module}_args)
          # keep REQUIRED in the arguments only if we were required ourself
          # "required-ness" is not transitive as far as CMake is concerned
          # (i.e. if an optional package requests a package to be required,
          # the build will fail if it's not found)
	  string (REPLACE "REQUIRED" "${_${module}_required}" _args_req "${_${module}_args}")
          if(_dep MATCHES "opm-")
            set(deplist ${_dep})
            string(STRIP "${_dep}" _dep)
            string(REPLACE " " ";" deplist "${_dep}")
            list(GET deplist 0 depname)
            create_module_dir_var(${depname})
          endif()
	  find_and_append_package_to (${module} ${_${module}_args} ${_${module}_quiet})
	  list (GET _${module}_args 0 _name_only)
	  string (TOUPPER "${_name_only}" _NAME_ONLY)
	  string (REPLACE "-" "_" _NAME_ONLY "${_NAME_ONLY}")
	  # check manually if it was found if REQUIRED; otherwise poison the
	  # dependency list which is checked later (so that it will fail)
	  if (("${_${module}_args}" MATCHES "REQUIRED") AND NOT (${_name_only}_FOUND OR ${_NAME_ONLY}_FOUND))
		list (APPEND ${module}_ALL_PREREQS "${_name_only}-NOTFOUND")
	  endif ()
	else ()
	  message (WARNING "Empty dependency in find module for ${module} (check for trailing semi-colon)")
	endif ()
  endforeach (_dep)

  # tidy the lists before returning them
  remove_dup_deps (${module})

  # these defines are used in dune/${module} headers, and should be put
  # in config.h when we include those
  foreach (_var IN ITEMS ${conf})
    # massage the name to remove source code formatting
    string (REGEX REPLACE "^[\n\t\ ]+" "" _var "${_var}")
    string (REGEX REPLACE "[\n\t\ ]+$" "" _var "${_var}")
    list (APPEND ${module}_CONFIG_VARS ${_var})
  endforeach (_var)

  # these are the defines that should be set when compiling
  # without config.h
  config_cmd_line (${module}_CMD_CONFIG ${module}_CONFIG_VARS)

  # This variable is used by UseDuneVer
  list(GET ${module}_INCLUDE_DIRS 0 ${module}_INCLUDE_DIR)
  # print everything out if we're asked to
  if (${module}_DEBUG)
	debug_find_vars (${module})
  endif (${module}_DEBUG)
endmacro ()

# print all variables defined by the above macro
function (debug_find_vars module)
  message (STATUS "${module}_FOUND        = ${${module}_FOUND}")
  message (STATUS "${module}_INCLUDE_DIRS = ${${module}_INCLUDE_DIRS}")
  message (STATUS "${module}_LIBRARIES    = ${${module}_LIBRARIES}")
  message (STATUS "${module}_DEFINITIONS  = ${${module}_DEFINITIONS}")
  message (STATUS "${module}_CONFIG_VARS  = ${${module}_CONFIG_VARS}")
  message (STATUS "${module}_LINKER_FLAGS = ${${module}_LINKER_FLAGS}")
  string (TOUPPER ${module} MODULE)
  string (REPLACE "-" "_" MODULE ${MODULE})
  message (STATUS "HAVE_${MODULE}         = ${HAVE_${MODULE}}")
endfunction (debug_find_vars module)

# generate a command-line that can be used to pass variables before
# config.h is available (such as probe tests). varname is the *name*
# of the variable to receive the result, defs is a list of the *names*
# which should be passed
function (config_cmd_line varname defs)
  # process each variable
  foreach (_var IN LISTS ${defs})
	# only generate an entry if the define was actually set
	if ((DEFINED ${_var}) AND (NOT "${${_var}}" STREQUAL ""))
	  # add command-line option to define this variable
	  list (APPEND _cmdline "-D${_var}=${${_var}}")
	endif ((DEFINED ${_var}) AND (NOT "${${_var}}" STREQUAL ""))
  endforeach (_var)
  # return the resulting command-line options for defining vars
  set (${varname} "${_cmdline}" PARENT_SCOPE)
endfunction (config_cmd_line)
