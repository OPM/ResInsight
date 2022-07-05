# Poor man's FindTBB that will create the CMake targets
# used by DUNE.
# If the TBB version is new enough it will ship its own
# TBBConfig.cmake and we are good
find_package(TBB QUIET CONFIG)

if(NOT TBB_FOUND)
  # Fall back to using pkgconfig
  find_package(PkgConfig QUIET)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(PkgConfigTBB IMPORTED_TARGET GLOBAL tbb QUIET)
    if(NOT TARGET PkgConfig::PkgConfigTBB)
      # workaround bug in old FindPkgConfig.cmake which adds
      # pkgcfg_lib_PkgConfigTBB_atomic-NOTFOUND  because it cannot
      # find the atomic lib of the compiler (not in platforms default
      # library path. It will therefore not create the target and we
      # try that manually.
      string(REPLACE ";pkgcfg_lib_PkgConfigTBB_atomic-NOTFOUND" "" _find_tbb_libs "${PkgConfigTBB_LINK_LIBRARIES}")
      if(_find_tbb_libs)
	add_library(PkgConfig::PkgConfigTBB INTERFACE IMPORTED GLOBAL)
	set_property(TARGET PkgConfig::PkgConfigTBB PROPERTY
	  INTERFACE_LINK_LIBRARIES "${_find_tbb_libs}")
      endif()
    endif()
    if(TARGET PkgConfig::PkgConfigTBB)
      if(NOT TARGET TBB::tbb)
	message(STATUS "Found TBB library using pkg config")
	add_library(TBB::tbb ALIAS PkgConfig::PkgConfigTBB)
      endif()
    endif()
  endif(PKG_CONFIG_FOUND)
else()
  message(STATUS "Found TBB library using config mode")
endif(NOT TBB_FOUND)
