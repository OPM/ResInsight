# - Create config.h based on a list of variables
#
# Synopsis:
#   configure_vars (FILE syntax filename verb varlist)
# where
#	syntax        CXX or CMAKE, depending on target
#   filename      Full path (including name) of config.h
#   verb          WRITE or APPEND if truncating or not
#   varlist       List of variable names that has been defined
#
# In addition, this function will define HAVE_CONFIG_H for the
# following compilations, (only) if the filename is "config.h".
#
# Example:
#   list (APPEND FOO_CONFIG_VARS
#     "/* bar library */"
#     "HAVE_BAR"
#     "HAVE_BAR_VERSION_2"
#     )
#   configure_vars (
#     FILE  CXX  ${PROJECT_BINARY_DIR}/config.h
#     WRITE ${FOO_CONFIG_VARS}
#     )

# Copyright (C) 2012 Uni Research AS
# This file is licensed under the GNU General Public License v3.0

function (configure_vars obj syntax filename verb)
  # this is just to make the syntax look like the build-in commands
  if (NOT ("X Y Z ${obj}" STREQUAL "X Y Z FILE" AND
		(("${verb}" STREQUAL "WRITE") OR ("${verb}" STREQUAL "APPEND"))))
	message (FATAL_ERROR "Syntax error in argument list")
  endif ()
  if (NOT (("${syntax}" STREQUAL "CXX") OR ("${syntax}" STREQUAL "CMAKE")))
	message (FATAL_ERROR "Invalid target syntax \"${syntax}\"")
  endif (NOT (("${syntax}" STREQUAL "CXX") OR ("${syntax}" STREQUAL "CMAKE")))
  
  # truncate the file if the verb was "WRITE"
  if (verb STREQUAL "WRITE")
	file (WRITE "${filename}" "")
  endif (verb STREQUAL "WRITE")
  
  # whenever we use this, we also signal to the header files that we
  # have "config.h". add this before any other files (known till now)
  # to avoid confusion from other configuration files.
  get_filename_component (_config_path "${filename}" PATH)
  get_filename_component (_config_file "${filename}" NAME)
  if ("${_config_file}" MATCHES "config\\.h(\\..+)?")
	add_definitions (-DHAVE_CONFIG_H=1)
	include_directories (BEFORE "${_config_path}")
  endif ("${_config_file}" MATCHES "config\\.h(\\..+)?")
  
  # only write the current value of each variable once
  set (_args ${ARGN})
  if (_args)
	list (REMOVE_DUPLICATES _args)
  endif (_args)
  
  # process each variable
  set (_prev_verbatim TRUE)
  foreach (_var IN LISTS _args)

	# massage the name to remove source code formatting
	string (REGEX REPLACE "^[\\n\\t\\ ]+" "" _var "${_var}")
	string (REGEX REPLACE "[\\n\\t\\ ]+$" "" _var "${_var}")
	
	# if the name of a variable has the syntax of a comments, write it
	# verbatim to the file; this can be used to create headings
	if ("X Y Z ${_var}" MATCHES "^X Y Z /[/*]")
	  if (NOT _prev_verbatim)
		file (APPEND "${filename}" "\n")
	  endif (NOT _prev_verbatim)
	  file (APPEND "${filename}" "${_var}\n")
	  set (_prev_verbatim TRUE)
	  
	else ()

	  # write a CMake statements that warns if the value has changed
	  if ("${syntax}" STREQUAL "CMAKE")
		set (_db "\${") # to avoid parsing problems
		# special case: if we have a truth variable HAVE_ and this is
		# either just defined (as is), or set to 1 explicitly, then both
		# of these count as "true", so put in a check that also accepts
		# both of these values.
		if (("${_var}" MATCHES "^HAVE_.*") AND
			 (("${${_var}}" STREQUAL "") OR ("${${_var}}" STREQUAL "1")))
		  set (_cond "(\"${_db}${_var}}\" STREQUAL \"\") OR (\"${_db}${_var}}\" STREQUAL \"1\")")
		else ()
		  set (_cond "\"${_db}${_var}}\" STREQUAL \"${${_var}}\"")
		endif ()
		file (APPEND "${filename}" "if (DEFINED ${_var} AND NOT (${_cond}))\n")
		file (APPEND "${filename}" "\tmessage (WARNING \"Incompatible value \\\"${_db}${_var}}\\\" of variable \\\"${_var}\\\"\")\n")
		file (APPEND "${filename}" "endif ()\n")
	  endif ()
	  
	  # check for empty variable; variables that are explicitly set to false
	  # is not included in this clause
	  if ((NOT DEFINED ${_var}) OR ("${${_var}}" STREQUAL ""))
		if ("${syntax}" STREQUAL "CMAKE")
		  file (APPEND "${filename}" "set (${_var})\n")
		else ("${syntax}" STREQUAL "CMAKE")
		  file (APPEND "${filename}" "/* #undef ${_var} */\n")
		endif ("${syntax}" STREQUAL "CMAKE")
	  else ((NOT DEFINED ${_var}) OR ("${${_var}}" STREQUAL ""))
		# write to file using the correct syntax
		if ("${syntax}" STREQUAL "CMAKE")
		  # escape backslash and double quote characters
		  string (REPLACE "\\" "\\\\" _quoted "${${_var}}")
		  string (REPLACE "\"" "\\\"" _quoted "${_quoted}")

		  file (APPEND "${filename}" "set (${_var} \"${_quoted}\")\n")
		else ("${syntax}" STREQUAL "CMAKE")
		  file (APPEND "${filename}" "#define ${_var} ${${_var}}\n")
		endif ("${syntax}" STREQUAL "CMAKE")
		
	  endif ((NOT DEFINED ${_var}) OR ("${${_var}}" STREQUAL ""))
	  set (_prev_verbatim FALSE)
	endif ()
  endforeach(_var)
endfunction (configure_vars obj syntax filename verb)
