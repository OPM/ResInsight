# - Get version control information from source tree
#
# Sets the following variables
#
#	VCS_SYSTEM      Currently, this will only be "git", or empty if
#	                no source code control system is found.
#
#	VCS_SHA1        Hash code of the last commit. If this is empty,
#	                then no source code repository was found.
#
#	VCS_DECOR       Characters that denotes any local modifications:
#	                "*" - Unstaged local changes
#	                "+" - Staged, but not committed, local changes
#	                "%" - Untracked local files
function (vcs_info)
  # if we haven't located git yet, then do it now
  if (NOT GIT_FOUND)
	find_package (Git)
  endif (NOT GIT_FOUND)

  # if git is not installed (unpacked tarball), then just state that
  # the version practically is unknown
  set (VCS_DECOR "")
  if (GIT_FOUND)
	set (VCS_SYSTEM "git")

	# assume that we have a .git subdirectory under the source directory;
	# if we have a bare repository, then we won't be able to build in there
	# and we won't be able to identify the git dir to use from only a work
	# tree, so we handle that like a regular unpacked tarball

	# exec_program is used because execute_process is buggy on common
	# platforms (notable CMake 2.8.7 in Ubuntu Precise 12.04)

	# get hash code
	exec_program (
	  ${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
	  ARGS rev-parse --short --verify HEAD
	  OUTPUT_VARIABLE VCS_SHA1
	  RETURN_VALUE has_sha
	  )

	# exec_program mashes together output and error
	if (NOT ${has_sha} EQUAL 0)
	  set (VCS_SHA1 "")
	endif (NOT ${has_sha} EQUAL 0)
	
	# only proceed if we actually found a source tree in there
	if (VCS_SHA1)
	  # check for unstaged local changes
	  exec_program (
		${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
		ARGS diff --no-ext-diff --quiet --exit-code
		RETURN_VALUE dirty
		OUTPUT_VARIABLE _dummy
		)
	  if (NOT ${dirty} EQUAL 0)
		list (APPEND VCS_DECOR "*")
	  endif (NOT ${dirty} EQUAL 0)

	  # check for staged local changes
	  exec_program (
		${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
		ARGS diff-index --no-ext-diff --cached --quiet --exit-code HEAD --
		RETURN_VALUE staged
		OUTPUT_VARIABLE _dummy
		)
	  if (NOT ${staged} EQUAL 0)
		list (APPEND VCS_DECOR "+")
	  endif (NOT ${staged} EQUAL 0)

	  # check for untracked files
	  exec_program (
		${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
		ARGS ls-files --others --exclude-standard
		OUTPUT_VARIABLE untracked
		)
	  if (untracked)
		list (APPEND VCS_DECOR "%")
	  endif (untracked)
	  
	  # convert list to regular string
	  string (REPLACE ";" "" VCS_DECOR "${VCS_DECOR}")
	endif (VCS_SHA1)
  else (GIT_FOUND)
	set (VCS_SYSTEM "")
	set (VCS_SHA1 "")
	set (VCS_DECOR "")
  endif (GIT_FOUND)

  # diagnostic output
  if (VCS_SYSTEM AND VCS_SHA1)
	message (STATUS "Source code repository: ${VCS_SYSTEM} ${VCS_SHA1}${VCS_DECOR}")
  else (VCS_SYSTEM AND VCS_SHA1)
	message (STATUS "Source code repository: not found!")
  endif (VCS_SYSTEM AND VCS_SHA1)

  # export to parent context
  set (VCS_SYSTEM "${VCS_SYSTEM}" PARENT_SCOPE)
  set (VCS_SHA1 "${VCS_SHA1}" PARENT_SCOPE)
  set (VCS_DECOR "${VCS_DECOR}" PARENT_SCOPE)
endfunction (vcs_info)
