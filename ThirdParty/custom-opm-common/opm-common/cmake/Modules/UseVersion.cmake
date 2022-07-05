# - Write version information into the source code
#
# Add an unconditional target to the Makefile which checks the current
# SHA of the source directory and write to a header file if and *only*
# if this has changed (thus we avoid unnecessary rebuilds). By having
# this in the Makefile, we get updated version information even though
# we haven't done any reconfiguring.
#
# The time it takes to probe the VCS for this information and write it
# to the miniature file in negligable.
#
# If the build type is Debug, then we only write a static version
# information as it gets tiresome to rebuild the project everytime one
# makes changes to any of the unit tests.

message("-- Writing version information to local header project-version.h")
string (TIMESTAMP build_timestamp "%Y-%m-%d at %H:%M:%S hrs")

string (TOUPPER "${CMAKE_BUILD_TYPE}" cmake_build_type_upper_)
set(OPM_BINARY_PACKAGE_VERSION "" CACHE STRING
  "Version of the binary Linux package built (will be printed in PRT file of flow if not empty)")
string(LENGTH "${OPM_BINARY_PACKAGE_VERSION}" _BINARY_PACKAGE_VERSION_LENGTH)
if (cmake_build_type_upper_ MATCHES DEBUG)
  file (WRITE "${PROJECT_BINARY_DIR}/project-version.h"
        "#ifndef OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
        "#define OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
        "#define PROJECT_VERSION_NAME \"${${project}_LABEL}\"\n"
        "#define PROJECT_VERSION_HASH \"debug\"\n"
        "#define PROJECT_VERSION \"${${project}_LABEL} (debug)\"\n"
        "#endif // OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
	)

  # Write header file with build timestamp
  file (WRITE "${PROJECT_BINARY_DIR}/project-timestamp.h"
      "#ifndef OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      "#define OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      "#define BUILD_TIMESTAMP \"${build_timestamp}\"\n"
      "#endif // OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      )
else ()
  if (NOT GIT_FOUND)
	find_package (Git)
  endif ()

  # if git is *still* not found means it is not present on the
  # system, so there is "no" way we can update the SHA. notice
  # that this is a slightly different version of the label than
  # above.
  if (NOT GIT_FOUND OR NOT EXISTS ${PROJECT_SOURCE_DIR}/.git)
    if(_BINARY_PACKAGE_VERSION_LENGTH GREATER 0)
      set(_PROJECT_VERSION_HASH "${OPM_BINARY_PACKAGE_VERSION}")
    else()
      set(_PROJECT_VERSION_HASH "unknown git version")
    endif()
    file (WRITE "${PROJECT_BINARY_DIR}/project-version.h"
      "#ifndef OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
      "#define OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
      "#define PROJECT_VERSION_NAME \"${${project}_LABEL}\"\n"
      "#define PROJECT_VERSION_HASH \"${_PROJECT_VERSION_HASH}\"\n"
      "#define PROJECT_VERSION \"${${project}_LABEL} (${_PROJECT_VERSION_HASH})\"\n"
      "#endif // OPM_GENERATED_OPM_VERSION_HEADER_INCLUDED\n"
      )
    # Write header file with build timestamp
    file (WRITE "${PROJECT_BINARY_DIR}/project-timestamp.h"
      "#ifndef OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n"
      "#define OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n")
    if (_BINARY_PACKAGE_VERSION_LENGTH EQUAL 0)
      file(APPEND "${PROJECT_BINARY_DIR}/project-timestamp.h"
	"#define BUILD_TIMESTAMP \"${build_timestamp}\"\n")
    endif()
    file(APPEND "${PROJECT_BINARY_DIR}/project-timestamp.h"
      "#endif // OPM_GENERATED_OPM_TIMESTAMP_HEADER_INCLUDED\n")
  else ()
	add_custom_target (update-version ALL
	  COMMAND ${CMAKE_COMMAND}
	  -DCMAKE_HOME_DIRECTORY=${CMAKE_HOME_DIRECTORY}
	  -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
	  -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
	  -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}
	  -DPROJECT_LABEL=${${project}_LABEL}
	  -P ${OPM_MACROS_ROOT}/cmake/Scripts/WriteVerSHA.cmake
	  COMMENT "Updating version information"
	  )

	# the target above gets built every time thanks to the "ALL" modifier,
	# but it must also be done before the main library so it can pick up
	# any changes it does.
	if (${project}_TARGET)
	  add_dependencies (${${project}_TARGET} update-version)
	endif ()
  endif ()
endif ()

# safety precaution: check that we don't have version number mismatch.

# first get the name of the module (e.g. "core")
set (_module_regexp "([^-]+)-(.*)")
string (REGEX REPLACE "${_module_regexp}" "\\1" _suite_name "${project}")
string (REGEX REPLACE "${_module_regexp}" "\\2" _module_name "${project}")

# if we have a version number it must be in this file, e.g. opm/core/version.h
set (_rel_ver_h "${${project}_DIR}/${_module_name}/version.h")
set (_version_h "${PROJECT_SOURCE_DIR}/${_rel_ver_h}")

# not all modules have version files, so only check if they do
if (EXISTS "${_version_h}")
  # uppercase versions which is used in the file
  string (TOUPPER "${_suite_name}" _suite_upper)
  string (TOUPPER "${_module_name}" _module_upper)

  # scan the files for version define for major version
  set (_major_regexp "#define[ ]+${_suite_upper}_${_module_upper}_VERSION_MAJOR[ ]+([0-9]*)")
  file (STRINGS "${_version_h}" _version_h_major REGEX "${_major_regexp}")
  string (REGEX REPLACE "${_major_regexp}" "\\1" _version_h_major "${_version_h_major}")

  # exactly the same, but minor version (making a macro is more lines...)
  set (_minor_regexp "#define[ ]+${_suite_upper}_${_module_upper}_VERSION_MINOR[ ]+([0-9]*)")
  file (STRINGS "${_version_h}" _version_h_minor REGEX "${_minor_regexp}")
  string (REGEX REPLACE "${_minor_regexp}" "\\1" _version_h_minor "${_version_h_minor}")

  # compare what we got from the file with what we have defined here
  if (NOT (("${_version_h_major}" EQUAL "${${project}_VERSION_MAJOR}")
      AND  ("${_version_h_minor}" EQUAL "${${project}_VERSION_MINOR}")))
	set (_proj_ver "${${project}_VERSION_MAJOR}.${${project}_VERSION_MINOR}")
	set (_file_ver "${_version_h_major}.${_version_h_minor}")
	message (AUTHOR_WARNING
	  "Version in build system (dune.module) is \"${_proj_ver}\", "
	  "but version in source (${_rel_ver_h}) is \"${_file_ver}\""
	  )
  endif ()
endif ()
