# - This script must be passed the following information
#
#	GIT_EXECUTABLE      Path to the Git executable
#	PROJECT_SOURCE_DIR  Path to the source directory
#	PROJECT_BINARY_DIR  Path to the build directory
#	PROJECT_LABEL       String that identifies the minor
#	                    version of the project, e.g. "2013.03"
#

# get hash code
exec_program (
  ${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
  ARGS rev-parse --short --verify HEAD
  OUTPUT_VARIABLE sha1
  RETURN_VALUE has_sha
  )

# exec_program unfortunately mashes together both output
# and error streams, so we must use the return code to make
# sure that we only get the output
if (NOT ${has_sha} EQUAL 0)
  set (sha1 "")
endif ()

# check for local changes
if (sha1)
  # unstaged
  exec_program (
	${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
	ARGS diff --no-ext-diff --quiet --exit-code
	RETURN_VALUE dirty
	OUTPUT_VARIABLE _dummy
	)

  # staged
  exec_program (
	${GIT_EXECUTABLE} ${PROJECT_SOURCE_DIR}
	ARGS diff-index --no-ext-diff --cached --quiet --exit-code HEAD --
	RETURN_VALUE staged
	OUTPUT_VARIABLE _dummy
	)

  # if we found any changes, then append an asterisk to
  # the SHA1 so we know that it cannot be trusted
  if (dirty OR staged)
	set (sha1 "${sha1}*")
  endif ()
endif ()

string (TIMESTAMP build_timestamp "%Y-%m-%d at %H:%M:%S hrs")

# write the content to a temporary file in a C compatible format
file (WRITE "${PROJECT_BINARY_DIR}/project-version.tmp"
      "#ifndef OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
      "#define OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
      "#define PROJECT_VERSION_NAME \"${PROJECT_LABEL}\"\n"
      "#define PROJECT_VERSION_HASH \"${sha1}\"\n"
      "#define PROJECT_VERSION \"${PROJECT_LABEL} (${sha1})\"\n"
      "#endif // OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
      )

# only commit this to source code if it actually changed. here
# we use execute_process instead of exec_program to avoid having
# it printed on the console every time
execute_process (COMMAND
  ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_BINARY_DIR}/project-version.tmp" "${PROJECT_BINARY_DIR}/project-version.h"
  )

# Write header file with build timestamp
file (WRITE "${PROJECT_BINARY_DIR}/project-timestamp.h"
      "#ifndef OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      "#define OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      "#define BUILD_TIMESTAMP \"${build_timestamp}\"\n"
      "#endif // OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      )
