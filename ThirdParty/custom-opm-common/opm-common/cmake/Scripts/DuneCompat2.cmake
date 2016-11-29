# - Emulate a rule to patch the Makefile, adding a line to the source
# tree and write a marker file indicating it is done.

set (base_dir ".")
set (marker_file "${base_dir}/CMakeFiles/marker")
set (makefile "${base_dir}/Makefile")

# if the Makefile has changed, then update it
if ("${makefile}" IS_NEWER_THAN "${marker_file}")
  # only add the string once, so it does not return multiple
  # results for the command line (will lead to syntax error)
  file (STRINGS "${makefile}" abs_top_srcdir_FOUND
	REGEX "^abs_top_srcdir = "
	)
  if (NOT abs_top_srcdir_FOUND)
	file (APPEND "${makefile}"
	  "abs_top_srcdir = ${CMAKE_HOME_DIRECTORY}\n"
	  )
  endif (NOT abs_top_srcdir_FOUND)
  # touch the marker so that we won't update the Makefile again
  execute_process (COMMAND
	${CMAKE_COMMAND} -E touch "${marker_file}"
	)
endif ("${makefile}" IS_NEWER_THAN "${marker_file}")
	
