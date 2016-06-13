# translate a list of libraries into a command-line that can be passed to the
# compiler/linker. first parameter is the name of the variable that will
# receive this list, the rest is considered the list of libraries
function (linker_cmdline what INTO outvar FROM)
  # if we are going to put these in regexps, we must escape period
  string (REPLACE "." "\\." esc_dl_pref "${CMAKE_SHARED_LIBRARY_PREFIX}")
  string (REPLACE "." "\\." esc_dl_suff "${CMAKE_SHARED_LIBRARY_SUFFIX}")
  string (REPLACE "." "\\." esc_ar_pref "${CMAKE_STATIC_LIBRARY_PREFIX}")
  string (REPLACE "." "\\." esc_ar_suff "${CMAKE_STATIC_LIBRARY_PREFIX}")

  # CMake loves absolute paths, whereas libtool won't have any of it!
  # (you get an error message about argument not parsed). translate each
  # of the libraries into a linker option
  set (deplib_list "")
  foreach (deplib IN LISTS ARGN)
	# starts with a hyphen already? then just add it
	string (SUBSTRING ${deplib} 0 1 dash)
	if (${dash} STREQUAL "-")
	  list (APPEND deplib_list ${deplib})
	else (${dash} STREQUAL "-")
	  # otherwise, parse the name into a directory and a name
	  get_filename_component (deplib_dir ${deplib} PATH)
	  get_filename_component (deplib_orig ${deplib} NAME)
	  string (REGEX REPLACE
		"^${esc_dl_pref}(.*)${esc_dl_suff}$"
		"\\1"
		deplib_name
		${deplib_orig}
		)
	  string (REGEX REPLACE
		"^${esc_ar_pref}(.*)${esc_ar_suff}$"
		"\\1"
		deplib_name
		${deplib_name}
		)
	  # directory and name each on their own; this is somewhat
	  # unsatisfactory because it may be that a system dir is specified
	  # by an earlier directory and you start picking up libraries from
	  # there instead of the "closest" path here. also, the soversion
	  # is more or less lost. remove system default path, to lessen the
	  # chance that we pick the wrong library
	  if (NOT ((deplib_dir STREQUAL "/usr/lib") OR
			   (deplib_dir STREQUAL "/usr/${CMAKE_INSTALL_LIBDIR}")))
		   list (APPEND deplib_list "-L${deplib_dir}")
	  endif (NOT ((deplib_dir STREQUAL "/usr/lib") OR
			      (deplib_dir STREQUAL "/usr/${CMAKE_INSTALL_LIBDIR}")))
	  # if there was no translation of the name, the library is named
	  # unconventionally (.so.3gf, I'm looking at you), so pass this
	  # name unmodified to the linker switch
	  if (deplib_orig STREQUAL deplib_name)
		list (APPEND deplib_list "-l:${deplib_orig}")
	  else (deplib_orig STREQUAL deplib_name)
		list (APPEND deplib_list "-l${deplib_name}")
	  endif (deplib_orig STREQUAL deplib_name)
	endif (${dash} STREQUAL "-")
  endforeach (deplib)
  # caller determines whether we want it returned as a list or a string
  if ("${what}" STREQUAL "LIST")
	set (${outvar} ${deplib_list})
  else ("${what}" STREQUAL "LIST")
	set (${outvar} "${deplib_list}")
	string (REPLACE ";" " " ${outvar} "${${outvar}}")
  endif ("${what}" STREQUAL "LIST")
  set (${outvar} "${${outvar}}" PARENT_SCOPE)
endfunction (linker_cmdline what INTO outvar FROM)

function (configure_la name target)
  if (NOT (UNIX OR MSYS OR MINGW))
	return ()
  endif (NOT (UNIX OR MSYS OR MINGW))
  
  # these generic variables are initialized from the project info
  set (current "${${name}_VERSION_MAJOR}")
  set (age "${${name}_VERSION_MINOR}")
  set (inherited_linker_flags "${${name}_LINKER_FLAGS}")
  set (dependency_libs "${${name}_LIBRARIES}")

  # translate list of libraries to command line
  linker_cmdline (LIST INTO dependency_libs FROM ${dependency_libs})
  
  # convert from CMake list (i.e. semi-colon separated)
  string (REPLACE ";" " " inherited_linker_flags "${inherited_linker_flags}")
  string (REPLACE ";" " " dependency_libs "${dependency_libs}")

  # this is the preferred installation path
  set (libdir "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

  # ${name}_LIBRARY_TYPE is either SHARED or STATIC
  if (${name}_LIBRARY_TYPE STREQUAL "SHARED")
	set (libprefix "${CMAKE_SHARED_LIBRARY_PREFIX}")
	set (libsuffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set (libname "${CMAKE_SHARED_LIBRARY_PREFIX}${target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	# only Unix has soversion in library names
	if (UNIX)
	  set (dlname "${libname}.${current}")
	  set (library_names "${libname}.${current}.${age} ${libname}.${current} ${libname}")
	else (UNIX)
	  set (dlname "${libname}")
	  set (library_names "${libname}")
	endif (UNIX)
	set (old_library "")
  else (${name}_LIBRARY_TYPE STREQUAL "SHARED")
	set (dlname "")
	set (library_names "")
	set (old_library "${CMAKE_STATIC_LIBRARY_PREFIX}${target}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif (${name}_LIBRARY_TYPE STREQUAL "SHARED")

  # get the version of libtool installed on the system; this is
  # necessary because libtool checks that the file contains its own
  # signature(!)
  if (NOT libtool_MAIN)
	find_file (
	  libtool_MAIN
	  ltmain.sh
	  PATHS /usr
	  PATH_SUFFIXES share/libtool/config/
	  DOC "Location of libtool"
	  )
	mark_as_advanced (libtool_MAIN)
	# notify the user if it not found after we explicitly searched
	if (NOT libtool_MAIN)
	  message (STATUS "Libtool not found!")	  
	endif (NOT libtool_MAIN)
  endif (NOT libtool_MAIN)
  if (libtool_MAIN)
	file (STRINGS
	  ${libtool_MAIN}
	  ltversion_STRING
	  REGEX "^VERSION=\".*\""
	  )
  endif (libtool_MAIN)
  if (ltversion_STRING)	
	string (REGEX REPLACE
	  "^VERSION=\"?(.*)\"?"
	  "\\1"
	  ltversion
	  ${ltversion_STRING}
	  )
  endif (ltversion_STRING)

  # assume that we are in cmake/Modules, and that the template have been
  # put in cmake/Templates. we cannot use CMAKE_CURRENT_LIST_DIR because
  # this is in a function, and we cannot know who's calling us
  set (templ_dir "${OPM_MACROS_ROOT}/cmake/Templates")

  
  # only write an .la if libtool is found; otherwise we have no use
  # for it.
  if (ltversion)
	set (la_file "lib${target}.la")
	message (STATUS "Writing libtool archive for ${target}")
	configure_file (
	  ${templ_dir}/la.in
	  ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${la_file}
	  @ONLY@
	  )
  else (ltversion)
	set (la_file "")
  endif (ltversion)

  # return this variable to the caller
  if (ARGV2)
	set (${ARGV2} "${la_file}" PARENT_SCOPE)
  endif (ARGV2)
endfunction (configure_la target)
