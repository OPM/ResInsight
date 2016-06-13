# - Remove a directory if and only if it contains no files
#
# Pass the name of the directory as the DIR variable

if (DIR)
  # check if empty
  file (GLOB_RECURSE files "${DIR}/*")
  
  # remove only if
  if (NOT files)
	execute_process (COMMAND
	  ${CMAKE_COMMAND} -E remove_directory "${DIR}"
	  )
  endif (NOT files)
endif (DIR)
