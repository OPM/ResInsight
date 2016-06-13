# - Find version of a DUNE package
#
# Synopsis:
#
#	find_dune_version (suite module)
#
# where:
# 	suite   Name of the suite; this should always be "dune"
# 	module  Name of the module, e.g. "common"
#
# Finds the content of DUNE_${MODULE}_VERSION_{MAJOR,MINOR,REVISION}
# from the installation.
#
# Add these variables to ${project}_CONFIG_IMPL_VARS in CMakeLists.txt
# if you need these in the code.

include (UseMultiArch)

function (find_dune_version suite module)
  # the _ROOT variable may or may not be set, but the include
  # variable should always be; get the prefix from the header path
  # if we have a multilib installation where the package maintainer
  # have installed it in e.g. /usr/include/dune-2.2/dune/istl, then
  # stash this extra indirection and add it back later in lib/
  set (_inc_path "${${suite}-${module}_INCLUDE_DIR}")
  file (TO_CMAKE_PATH _inc_path "${_inc_path}")
  set (_multilib_regexp "(.*)/include(/${suite}[^/]+)?")
  if (_inc_path MATCHES "${_multilib_regexp}")
	set (_orig_inc "${_inc_path}")
	string (REGEX REPLACE "${_multilib_regexp}" "\\1" _inc_path "${_orig_inc}")
	# only get the second group if it is really there (there is
	# probably a better way to do this in CMake)
	if ("${_inc_path}/include" STREQUAL "${_orig_inc}")
	  set (_multilib "")
	else ()
	  string (REGEX REPLACE "${_multilib_regexp}" "\\2" _multilib "${_orig_inc}")
	endif ()
  else ()
	set (_multilib "")
  endif ()

  # some modules does not have a library, use the directory of the
  # header files to find what would be the library dir.
  # note that when we refer to a build tree, then the libraries always
  # go into lib/, but we don't care about that because in that case,
  # dune.module isn't in the lib/ directory anyway but must be retrieved
  # from the source. hence, we only have to worry about the library
  # directory of a system installation here.
  if (NOT ${suite}-${module}_LIBRARY)
	# this suffix is gotten from UseMultiArch.cmake
	set (_lib_path "${_inc_path}/${CMAKE_INSTALL_LIBDIR}")
  else ()
	get_filename_component (_lib_path "${${suite}-${module}_LIBRARY}" PATH)
  endif ()

  # if we have a source tree, dune.module is available there
  set (_dune_mod "${_inc_path}/dune.module")
  if (NOT EXISTS "${_dune_mod}")
	set (_last_dune_mod_src "${_dune_mod}")
	set (_dune_mod "")
  endif ()

  if (NOT _dune_mod)
	# look for the build tree; if we found the library, then the
	# dune.module file should be in a sub-directory  
	get_filename_component (_immediate "${_lib_path}" NAME)
	if ("${_immediate}" STREQUAL ".libs")
	  # remove autotools internal path
	  get_filename_component (_lib_path "${_lib_path}" PATH)
	endif ()
	get_filename_component (_immediate "${_lib_path}" NAME)
	if ("${_immediate}" STREQUAL "${CMAKE_LIBRARY_ARCHITECTURE}")
	  # remove multi-arch part of the library path to get parent
	  get_filename_component (_lib_path "${_lib_path}" PATH)
	endif ()
	get_filename_component (_immediate "${_lib_path}" NAME)	
	if (("${_immediate}" STREQUAL "${CMAKE_INSTALL_LIBDIR}")
		OR ("${_immediate}" STREQUAL "lib")
		OR ("${_immediate}" STREQUAL "${LIBDIR_MULTIARCH_UNAWARE}"))
	  # remove library part of the path; this also undo the suffix
	  # we added if we used the library as a standin
	  get_filename_component (_lib_path "${_lib_path}" PATH)
	endif ()
	# from this point on, _lib_path does not contain an architecture-
	# specific component anymore; dune.module is always put in straight
	# noarch lib/ since it does not contain any paths to binaries
	set (_suffix "${_multilib}/dunecontrol/${suite}-${module}/dune.module")
	set (_dune_mod "${_lib_path}/${LIBDIR_MULTIARCH_UNAWARE}${_suffix}")
	if (NOT EXISTS "${_dune_mod}")
	  set (_last_dune_mod_bld "${_dune_mod}")
	  # one more try, if we have a private install, then it doesn't use
	  # e.g. lib64 but always lib (!)
	  if ("${LIBDIR_MULTIARCH_UNAWARE}" STREQUAL "lib")
		set (_dune_mod "")
	  else ()
		set (_dune_mod "${_lib_path}/lib${_suffix}")
		if (NOT EXISTS "${_dune_mod}")
		  set (_last_dune_mod_pri "${_dune_mod}")
		  # use the name itself as a flag for whether it was found or not
		  set (_dune_mod "")
		endif ()
	  endif ()
	endif ()
  endif ()

  # if it is not available, it may make havoc having empty defines in the source
  # code later, so we bail out early
  if (NOT _dune_mod)
	if (${suite}-${module}_FOUND)
	  set (_searched_paths "\"${_last_dune_mod_src}\"")
	  if (NOT ("${_last_dune_mod_bld}" STREQUAL ""))
		set (_searched_paths "either ${_searched_paths} or \"${_last_dune_mod_bld}\"")
	  endif ()
	  if (NOT ("${_last_dune_mod_pri}" STREQUAL ""))
		set (_searched_paths "${_searched_paths} or \"${_last_dune_mod_pri}\"")
	  endif ()
	  message (FATAL_ERROR "Failed to locate dune.module for ${suite}-${module} (looking for ${_searched_paths})")
	else ()
	  return ()
	endif ()
  endif ()

  # parse the file for the Version: field
  set (_ver_regexp "[ ]*Version:[ ]*([0-9]+)\\.([0-9]+)(.*)")
  file (STRINGS "${_dune_mod}" _ver_field REGEX "${_ver_regexp}")
  string (REGEX REPLACE "${_ver_regexp}" "\\1" _major "${_ver_field}")
  string (REGEX REPLACE "${_ver_regexp}" "\\2" _minor "${_ver_field}")
  string (REGEX REPLACE "${_ver_regexp}" "\\3" _revision "${_ver_field}")

  # revision may or may not be there
  set (_rev_regexp "\\.([0-9]+).*")
  if (_revision MATCHES "${_rev_regexp}")
	string (REGEX REPLACE "${_rev_regexp}" "\\1" _revision "${_revision}")
  else ()
	set (_revision "0")
  endif ()

  # generate variable for what we have found
  string (TOUPPER "${suite}" _SUITE)
  string (TOUPPER "${module}" _MODULE)
  string (REPLACE "-" "_" _MODULE "${_MODULE}")
  if ((NOT DEFINED ${_SUITE}_${_MODULE}_VERSION_MAJOR) AND
	  (NOT DEFINED ${_SUITE}_${_MODULE}_VERSION_MINOR) AND
	  (NOT DEFINED ${_SUITE}_${_MODULE}_VERSION_REVISION))
	set (${_SUITE}_${_MODULE}_VERSION_MAJOR "${_major}" PARENT_SCOPE)
	set (${_SUITE}_${_MODULE}_VERSION_MINOR "${_minor}" PARENT_SCOPE)
	set (${_SUITE}_${_MODULE}_VERSION_REVISION "${_revision}" PARENT_SCOPE)
  endif ()

  # print the version number we detected in the configuration log
  message (STATUS "Version ${_major}.${_minor}.${_revision} of ${suite}-${module} from ${_dune_mod}")  
endfunction (find_dune_version suite module)
