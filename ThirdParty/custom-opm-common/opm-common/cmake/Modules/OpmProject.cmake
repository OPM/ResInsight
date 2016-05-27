# - Helper routines for opm-core like projects

include (LibtoolArchives) # linker_cmdline

# convert a list back to a command-line string
function (unseparate_args var_name prefix value)
  separate_arguments (value)
  foreach (item IN LISTS value)
	set (prefixed_item "${prefix}${item}")
	if (${var_name})
	  set (${var_name} "${${var_name}} ${prefixed_item}")
	else (${var_name})
	  set (${var_name} "${prefixed_item}")
	endif (${var_name})
  endforeach (item)
  set (${var_name} "${${var_name}}" PARENT_SCOPE)
endfunction (unseparate_args var_name prefix value)

# wrapper to set variables in pkg-config file
function (configure_pc_file name source dest prefix libdir includedir)
  # escape set of standard strings
  unseparate_args (includes "-I" "${${name}_INCLUDE_DIRS}")
  unseparate_args (defs "" "${${name}_DEFINITIONS}")
  linker_cmdline (STRING INTO libs FROM ${${name}_LIBRARIES})

  # necessary to make these variables visible to configure_file
  set (name "${${name}_NAME}")
  set (description "${${name}_DESCRIPTION}")
  set (major "${${name}_VERSION_MAJOR}")
  set (minor "${${name}_VERSION_MINOR}")
  set (target "${${name}_LIBRARY}")
  linker_cmdline (STRING INTO target from ${target})

  configure_file (${source} ${dest} @ONLY)
endfunction (configure_pc_file name source dist prefix libdir includedir)

function (configure_cmake_file name variant version)
  # declarative list of the variable names that are used in the template
  # and that must be defined in the project to be exported
  set (variable_suffices
	DESCRIPTION
	VERSION
	DEFINITIONS
	INCLUDE_DIRS
	LIBRARY_DIRS
	LINKER_FLAGS
	CONFIG_VARS
	LIBRARY
	LIBRARIES
	TARGET
	)

  # set these variables temporarily (this is in a function scope) so
  # they are available to the template (only)
  foreach (suffix IN LISTS variable_suffices)
	set (opm-project_${suffix} "${${name}_${suffix}}")
  endforeach (suffix)
  set (opm-project_NAME "${${name}_NAME}")

  # make the file substitutions
  configure_file (
	${template_dir}/opm-project-config${version}.cmake.in
	${PROJECT_BINARY_DIR}/${${name}_NAME}-${variant}${version}.cmake
	@ONLY
	)
endfunction (configure_cmake_file name)

# installation of CMake modules to help user programs locate the library
function (opm_cmake_config name)
  # assume that the template is located in cmake/Templates (cannot use
  # the current directory since this is in a function and the directory
  # at runtime not at definition will be used
  set (template_dir "${OPM_MACROS_ROOT}/cmake/Templates")

  # write configuration file to locate library
  set(OPM_PROJECT_EXTRA_CODE ${OPM_PROJECT_EXTRA_CODE_INTREE})
  configure_cmake_file (${name} "config" "")
  configure_cmake_file (${name} "config" "-version")
  configure_vars (
	FILE CMAKE "${PROJECT_BINARY_DIR}/${${name}_NAME}-config.cmake"
	APPEND "${${name}_CONFIG_VARS}"
	)

  # config-mode .pc file; use this to find the build tree
  configure_pc_file (
	${name}
	${template_dir}/opm-project.pc.in
	${PROJECT_BINARY_DIR}/${${name}_NAME}.pc
	${PROJECT_BINARY_DIR}
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
	${PROJECT_SOURCE_DIR}
	)

  # replace the build directory with the target directory in the
  # variables that contains build paths
  string (REPLACE
	"${PROJECT_SOURCE_DIR}"
	"${CMAKE_INSTALL_PREFIX}/include${${name}_VER_DIR}"
	${name}_INCLUDE_DIRS
	"${${name}_INCLUDE_DIRS}"
	)
  string (REPLACE
	"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${${name}_VER_DIR}"
	${name}_LIBRARY
	"${${name}_LIBRARY}"
	)
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${${name}_VER_DIR}"
	)
  # create a config mode file which targets the install directory instead
  # of the build directory (using the same input template)
  set(OPM_PROJECT_EXTRA_CODE ${OPM_PROJECT_EXTRA_CODE_INSTALLED})
  configure_cmake_file (${name} "install" "")
  configure_vars (
	FILE CMAKE "${PROJECT_BINARY_DIR}/${${name}_NAME}-install.cmake"
	APPEND "${${name}_CONFIG_VARS}"
	)
  # this file gets copied to the final installation directory
  install (
	FILES ${PROJECT_BINARY_DIR}/${${name}_NAME}-install.cmake
	DESTINATION share/cmake${${name}_VER_DIR}/${${name}_NAME}
	RENAME ${${name}_NAME}-config.cmake
	)
  # assume that there exists a version file already
  install (
	FILES ${PROJECT_BINARY_DIR}/${${name}_NAME}-config-version.cmake
	DESTINATION share/cmake${${name}_VER_DIR}/${${name}_NAME}
	)

  # find-mode .pc file; use this to locate system installation
  configure_pc_file (
	${name}
	${template_dir}/opm-project.pc.in
	${PROJECT_BINARY_DIR}/${${name}_NAME}-install.pc
	${CMAKE_INSTALL_PREFIX}
	${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${${name}_VER_DIR}
	${CMAKE_INSTALL_PREFIX}/include${${name}_VER_DIR}
	)

  # put this in the right system location; if we have binaries then it
  # should go in the arch-specific lib/ directory, otherwise use the
  # common/noarch lib/ directory (these targets come from UseMultiArch)
  if (${name}_TARGET)
	set (_pkg_dir ${CMAKE_INSTALL_LIBDIR})
  else ()
	set (_pkg_dir ${LIBDIR_MULTIARCH_UNAWARE})
  endif ()
  install (
	FILES ${PROJECT_BINARY_DIR}/${${name}_NAME}-install.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/${_pkg_dir}/pkgconfig${${name}_VER_DIR}/
	RENAME ${${name}_NAME}.pc
	)
endfunction (opm_cmake_config name)
