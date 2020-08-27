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

function (find_dune_version suite module)
  # CMake's find_package will set <package>_VERSION_(MAJOR|MINOR|REVISION)
  # we simply rely on that.
  # generate variable for what we have found
  string (TOUPPER "${suite}" _SUITE)
  string (TOUPPER "${module}" _MODULE)
  string (REPLACE "-" "_" _MODULE "${_MODULE}")
  set (${_SUITE}_${_MODULE}_VERSION_MAJOR "${${suite}-${module}_VERSION_MAJOR}" PARENT_SCOPE)
  set (${_SUITE}_${_MODULE}_VERSION_MINOR "${${suite}-${module}_VERSION_MINOR}" PARENT_SCOPE)
  set (${_SUITE}_${_MODULE}_VERSION_REVISION "${${suite}-${module}_VERSION_PATCH}" PARENT_SCOPE)

  if( ${suite}-${module}_FOUND )
    # print the version number we detected in the configuration log
    message (STATUS "Version ${${suite}-${module}_VERSION_MAJOR}.${${suite}-${module}_VERSION_MINOR}.${${suite}-${module}_VERSION_PATCH} of ${suite}-${module} from ${${suite}-${module}_DIR}")
  endif()
endfunction (find_dune_version suite module)
