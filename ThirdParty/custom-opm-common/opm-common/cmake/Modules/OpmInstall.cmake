# - Installation macro
#
# Set up installation targets for the binary library. The following
# suffices must be defined for the prefix passed as parameter:
#
# _NAME             Name of the library
# _HEADERS          List of header files to install
# _TARGET           CMake target which builds the library
# _LIBRARY_TYPE     Static or shared library
# _DEBUG            File containing debug symbols
include (UseMultiArch)

macro (opm_install opm)
  foreach (_hdr IN LISTS ${opm}_HEADERS)
	get_filename_component (_dir ${_hdr} PATH)
	file (RELATIVE_PATH _rel_dir "${PROJECT_SOURCE_DIR}" "${_dir}")
	install (
	  FILES ${_hdr}
	  DESTINATION include${${opm}_VER_DIR}/${_rel_dir}
	  )
  endforeach (_hdr)
  install (
	TARGETS ${${opm}_TARGET}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${${opm}_VER_DIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${${opm}_VER_DIR}
	)
  # only /usr/lib/debug seems to be searched for debug info; if we have
  # write access to that directory (package installation), then default
  # to use it; otherwise put the debug files together with the library
  # (local installation). everything can be overridden by the option.
  if (CMAKE_INSTALL_PREFIX STREQUAL "/usr")
	set (_sys_dbg_def ON)
  else (CMAKE_INSTALL_PREFIX STREQUAL "/usr")
	set (_sys_dbg_def OFF)
  endif (CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  option (SYSTEM_DEBUG "Put .debug files in GDB debug file directory" ${_sys_dbg_def})
  set (DEBUG_FILE_DIRECTORY /usr/lib/debug CACHE LOCATION "GDB debug file directory")
  mark_as_advanced (DEBUG_FILE_DIRECTORY)
  if (SYSTEM_DEBUG AND NOT APPLE)
	set (_dbg_prefix "${DEBUG_FILE_DIRECTORY}/")
  else (SYSTEM_DEBUG AND NOT APPLE)
	set (_dbg_prefix "")
  endif (SYSTEM_DEBUG AND NOT APPLE)
  # static libraries don't have their debug info stripped, so there is
  # only a separate file when we are building shared objects
  if (${opm}_LIBRARY_TYPE STREQUAL "SHARED" AND ${opm}_TARGET AND ${opm}_DEBUG)
	# on MacOS X, debug files are actually bundles (directories)
	if (APPLE)
	  set (_dbg_type DIRECTORY)
	else ()
	  set (_dbg_type FILES)
	endif ()
	install (
	  ${_dbg_type} ${PROJECT_BINARY_DIR}/${${opm}_DEBUG}
	  DESTINATION ${_dbg_prefix}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${${opm}_VER_DIR}
	  )
  endif (${opm}_LIBRARY_TYPE STREQUAL "SHARED" AND ${opm}_TARGET AND ${opm}_DEBUG)
  # note that the DUNE parts that looks for dune.module is currently (2013-09) not
  # multiarch-aware and will thus put in lib64/ on RHEL and lib/ on Debian
  install (
	FILES ${PROJECT_SOURCE_DIR}/dune.module
	DESTINATION ${LIBDIR_MULTIARCH_UNAWARE}${${opm}_VER_DIR}/dunecontrol/${${opm}_NAME}
	)
endmacro (opm_install opm)
