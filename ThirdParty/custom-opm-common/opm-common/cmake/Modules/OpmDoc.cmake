# - Setup documentation
#
# Assumes that a Doxyfile template is located in the project root
# directory, and that all documentation is going to be generated
# into its own Documentation/ directory. It will also generate an
# installation target for the documentation (not built by default)
#
# Requires the following variables to be set:
# ${opm}_NAME                Name of the project
#
# Output the following variables:
# ${opm}_STYLESHEET_COPIED   Location of stylesheet to be removed in distclean

macro (opm_doc opm doxy_dir)
  # combine the template with local customization
  file (READ ${OPM_MACROS_ROOT}/cmake/Templates/Doxyfile _doxy_templ)
  string (REPLACE ";" "\\;" _doxy_templ "${_doxy_templ}")
  if (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/Doxylocal)
	file (READ ${PROJECT_SOURCE_DIR}/${doxy_dir}/Doxylocal _doxy_local)
	string (REPLACE ";" "\\;" _doxy_local "${_doxy_local}")
  else (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/Doxylocal)
	set (_doxy_local)
  endif (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/Doxylocal)
  file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${doxy_dir})
  file (WRITE ${PROJECT_BINARY_DIR}/${doxy_dir}/Doxyfile.in ${_doxy_templ} ${_doxy_local})
  # set this generically named variable so even the custom file can be shared
  set (src_DIR "${${opm}_DIR}")

  # copy the doxygen layout XML file to the build directorie's doxygen
  # directory. if the source module ships with such a file it takes
  # precedence over the one shipped with the build system.
  if (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/DoxygenLayout.xml)
    file(COPY ${PROJECT_SOURCE_DIR}/${doxy_dir}/DoxygenLayout.xml DESTINATION ${PROJECT_BINARY_DIR}/${doxy_dir})
  else()
    file(COPY ${OPM_MACROS_ROOT}/cmake/Templates/DoxygenLayout.xml DESTINATION ${PROJECT_BINARY_DIR}/${doxy_dir})
  endif()

  # replace variables in this combined file
  configure_file (
	${PROJECT_BINARY_DIR}/${doxy_dir}/Doxyfile.in
	${PROJECT_BINARY_DIR}/${doxy_dir}/Doxyfile
	@ONLY
	)
  find_package (Doxygen)
  if (DOXYGEN_FOUND)
	add_custom_target (doc
	  COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/${doxy_dir}/Doxyfile
	  SOURCES ${PROJECT_BINARY_DIR}/${doxy_dir}/Doxyfile
	  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${doxy_dir}
	  COMMENT "Generating API documentation with Doxygen"
	  VERBATIM
	  )
	# distributions have various naming conventions; this enables the packager
	# to direct where the install target should put the documentation. the names
	# here are taken from GNUInstallDirs.cmake
	set (CMAKE_INSTALL_DATAROOTDIR "share" CACHE STRING "Read-only arch.-indep. data root")
	set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc${${opm}_VER_DIR}/${${opm}_NAME}" CACHE STRING "Documentation root")
	set (_formats html)
	foreach (format IN LISTS _formats)
	  string (TOUPPER ${format} FORMAT)
	  install (
		DIRECTORY ${PROJECT_BINARY_DIR}/${doxy_dir}/${format}
		DESTINATION ${CMAKE_INSTALL_DOCDIR}
		COMPONENT ${format}
		OPTIONAL
		)
	  # target to install just HTML documentation
	  add_custom_target (install-${format}
		COMMAND ${CMAKE_COMMAND} -DCOMPONENT=${format} -P cmake_install.cmake
		COMMENT Installing ${FORMAT} documentation
		VERBATIM
		)
	  # since the documentation is optional, it is not automatically built
	  add_dependencies (install-${format} doc)
	endforeach (format)
  endif (DOXYGEN_FOUND)
  
  # stylesheets must be specified with relative path in Doxyfile, or the
  # full path (to the source directory!) will be put in the output HTML.
  # thus, we'll need to copy the stylesheet to this path relative to where
  # Doxygen will be run (in the output tree)
  if ((NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR) AND (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/style.css))
	file (COPY ${PROJECT_SOURCE_DIR}/${doxy_dir}/style.css
	  DESTINATION ${PROJECT_BINARY_DIR}/${doxy_dir}
	  )
	set (${opm}_STYLESHEET_COPIED "${doxy_dir}/style.css")
  else ((NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR) AND (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/style.css))
	set (${opm}_STYLESHEET_COPIED "")
  endif ((NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR) AND (EXISTS ${PROJECT_SOURCE_DIR}/${doxy_dir}/style.css))
  
endmacro (opm_doc opm)
