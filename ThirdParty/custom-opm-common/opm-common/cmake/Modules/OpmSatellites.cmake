# - Build satellites that are dependent of main library

#
# Enumerate all source code in a "satellite" directory such as tests/,
# compile each of them and optionally set them as a test for CTest to
# run. They will be linked to the main library created by the project.
#
# The following suffices must be defined for the opm prefix passed as
# parameter:
#
#  _LINKER_FLAGS   Necessary flags to link with this library
#  _TARGET         CMake target which creates the library
#  _LIBRARIES      Other dependencies that must also be linked

# Synopsis:
#  opm_compile_satellites (opm satellite excl_all test_regexp)
#
# Parameters:
#  opm             Prefix of the variable which contain information
#                  about the library these satellites depends on, e.g.
#                  pass "opm-core" if opm-core_TARGET is the name of
#                  the target the builds this library. Variables with
#                  suffixes _TARGET and _LIBRARIES must exist.
#
#  satellite       Prefix of variable which contain the names of the
#                  files, e.g. pass "tests" if the files are in the
#                  variable tests_SOURCES. Variables with suffixes
#                  _DATAFILES, _SOURCES and _DIR should exist. This
#                  name is also used as name of the target that builds
#                  all these files.
#
#  excl_all        EXCLUDE_FROM_ALL if these targets should not be built by
#                  default, otherwise empty string.
#
#  test_regexp     Regular expression which picks the name of a test
#                  out of the filename, or blank if no test should be
#                  setup.
#
# Example:
#  opm_compile_satellites (opm-core test "" "^test_([^/]*)$")
#
macro (opm_compile_satellites opm satellite excl_all test_regexp)
  # if we are going to build the tests always, then make sure that
  # the datafiles are present too
  if (NOT (${excl_all} MATCHES "EXCLUDE_FROM_ALL"))
    set (_incl_all "ALL")
  else (NOT (${excl_all} MATCHES "EXCLUDE_FROM_ALL"))
    set (_incl_all "")
  endif (NOT (${excl_all} MATCHES "EXCLUDE_FROM_ALL"))

  # if a set of datafiles has been setup, pull those in
  add_custom_target (${satellite} ${_incl_all})
  if (${satellite}_DATAFILES)
    add_dependencies (${satellite} ${${satellite}_DATAFILES})
  endif (${satellite}_DATAFILES)

  # compile each of these separately
  foreach (_sat_FILE IN LISTS ${satellite}_SOURCES)
    if (NOT "${test_regexp}" STREQUAL "" AND NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
        continue()
    endif()
    get_filename_component (_sat_NAME "${_sat_FILE}" NAME_WE)
    add_executable (${_sat_NAME} ${excl_all} ${_sat_FILE})
    add_dependencies (${satellite} ${_sat_NAME})
    set_target_properties (${_sat_NAME} PROPERTIES
                                        LINK_FLAGS "${${opm}_LINKER_FLAGS_STR}")
    if(HAVE_DYNAMIC_BOOST_TEST)
      set_target_properties (${_sat_NAME} PROPERTIES
                             COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK)
    endif()
    # are we building a test? luckily, the testing framework doesn't
    # require anything else, so we don't have to figure out where it
    # should go in the library list
    if (NOT "${test_regexp}" STREQUAL "")
      set (_test_lib "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
    else (NOT "${test_regexp}" STREQUAL "")
      set (_test_lib "")
      add_static_analysis_tests(_sat_FILE ${opm}_INCLUDE_DIRS)
    endif (NOT "${test_regexp}" STREQUAL "")
    target_link_libraries (${_sat_NAME} ${${opm}_TARGET} ${${opm}_LIBRARIES} ${_test_lib})
    if (STRIP_DEBUGGING_SYMBOLS)
      strip_debug_symbols (${_sat_NAME} _sat_DEBUG)
      list (APPEND ${satellite}_DEBUG ${_sat_DEBUG})
    endif()

    # variable with regular expression doubles as a flag for
    # whether tests should be setup or not
    set(_sat_FANCY)
    if (NOT "${test_regexp}" STREQUAL "")
      foreach (_regexp IN ITEMS ${test_regexp})
        if ("${_sat_NAME}" MATCHES "${_regexp}")
          string (REGEX REPLACE "${_regexp}" "\\1" _sat_FANCY "${_sat_NAME}")
        elseif(NOT _sat_FANCY)
          set(_sat_FANCY ${_sat_NAME})
        endif()
      endforeach (_regexp)
      get_target_property (_sat_LOC ${_sat_NAME} LOCATION)
      # Run tests through mpi-run. Ubuntu 14.04 provided mpi libs will crash
      # in the MPI_Finalize() call otherwise.
      if(MPI_FOUND)
        set(_sat_LOC ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 1 ${_sat_LOC})
      endif()
      if (CMAKE_VERSION VERSION_LESS "2.8.4")
        add_test (NAME ${_sat_FANCY}
                  COMMAND ${CMAKE_COMMAND} -E chdir "${PROJECT_BINARY_DIR}/${${satellite}_DIR}" ${_sat_LOC})
      else (CMAKE_VERSION VERSION_LESS "2.8.4")
      add_test (${_sat_FANCY} ${_sat_LOC})
      # run the test in the directory where the data files are
      set_tests_properties (${_sat_FANCY} PROPERTIES
                            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${${satellite}_DIR})
      endif (CMAKE_VERSION VERSION_LESS "2.8.4")
      if(NOT TARGET test-suite)
        add_custom_target(test-suite)
      endif()
      add_dependencies(test-suite "${_sat_NAME}")
    endif(NOT "${test_regexp}" STREQUAL "")

    # if this program on the list of files that should be distributed?
    # we check by the name of the source file
    list (FIND ${satellite}_SOURCES_DIST "${_sat_FILE}" _is_util)
    if (NOT (_is_util EQUAL -1))
      install (TARGETS ${_sat_NAME} RUNTIME
               DESTINATION bin${${opm}_VER_DIR}/)
    endif (NOT (_is_util EQUAL -1))
  endforeach (_sat_FILE)
endmacro (opm_compile_satellites opm prefix)

# Synopsis:
#  opm_data (satellite target dirname files)
#
# provides these output variables:
#
#  ${satellite}_INPUT_FILES   List of all files that are copied
#  ${satellite}_DATAFILES     Name of target which copies these files
#
# Example:
#
#  opm_data (tests datafiles "tests/")
#
macro (opm_data satellite target dirname)
  # even if there are no datafiles, create the directory so the
  # satellite programs have a homedir to run in
  execute_process (COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/${dirname})

  # if ever huge test datafiles are necessary, then change this
  # into "create_symlink" (on UNIX only, apparently)
  set (make_avail "copy")

  # provide datafiles as inputs for the tests, by copying them
  # to a tests/ directory in the output tree (if different)
  set (${satellite}_INPUT_FILES)
  if (NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    foreach (input_datafile IN LISTS ${satellite}_DATA)
      if (IS_ABSOLUTE ${input_datafile})
        file (RELATIVE_PATH rel_datafile "${PROJECT_SOURCE_DIR}" ${input_datafile})
      else()
        set(rel_datafile ${input_datafile})
      endif()
      set (output_datafile "${PROJECT_BINARY_DIR}/${rel_datafile}")
      add_custom_command (OUTPUT ${output_datafile}
                          COMMAND ${CMAKE_COMMAND}
                          ARGS -E ${make_avail} ${input_datafile} ${output_datafile}
                          DEPENDS ${input_datafile}
                          VERBATIM)
      list (APPEND ${satellite}_INPUT_FILES "${output_datafile}")
    endforeach (input_datafile)
  endif(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)

  # setup a target which does all the copying
  set (${satellite}_DATAFILES "${target}")
  add_custom_target (${${satellite}_DATAFILES}
                     DEPENDS ${${satellite}_INPUT_FILES}
                     COMMENT "Making \"${satellite}\" data available in output tree")
  if(NOT TARGET test-suite)
    add_custom_target(test-suite)
  endif()
  add_dependencies(test-suite ${${satellite}_DATAFILES})
endmacro (opm_data satellite target dirname files)

# Add a single unit test (can be orchestrated by the 'ctest' command)
#
# Synopsis:
#       opm_add_test(TestName)
#
# Parameters:
#       TestName           Name of test
#       ONLY_COMPILE       Only build test but do not run it (optional)
#       DEFAULT_ENABLE_IF  Only enable by default if a given condition is true (optional)
#       ALWAYS_ENABLE      Force enabling test even if -DBUILD_TESTING=OFF was set (optional)
#       EXE_NAME           Name of test executable (optional, default: ./bin/${TestName})
#       CONDITION          Condition to enable test (optional, cmake code)
#       DEPENDS            Targets which the test depends on (optional)
#       DRIVER             The script which supervises the test (optional, default: ${OPM_TEST_DRIVER})
#       DRIVER_ARGS        The script which supervises the test (optional, default: ${OPM_TEST_DRIVER_ARGS})
#       TEST_ARGS          Arguments to pass to test's binary (optional, default: empty)
#       SOURCES            Source files for the test (optional, default: ${EXE_NAME}.cpp)
#       PROCESSORS         Number of processors to run test on (optional, default: 1)
#       TEST_DEPENDS       Other tests which must be run before running this test (optional, default: None)
#       LIBRARIES          Libraries to link test against (optional)
#       WORKING_DIRECTORY  Working directory for test (optional, default: ${PROJECT_BINARY_DIR})
#       CONFIGURATION      Configuration to add test to
#
# Example:
#
# opm_add_test(funky_test
#              ALWAYS_ENABLE
#              CONDITION FUNKY_GRID_FOUND
#              SOURCES tests/MyFunkyTest.cpp
#              LIBRARIES -lgmp -lm)
include(CMakeParseArguments)

macro(opm_add_test TestName)
  cmake_parse_arguments(CURTEST
                        "NO_COMPILE;ONLY_COMPILE;ALWAYS_ENABLE" # flags
                        "EXE_NAME;PROCESSORS;WORKING_DIRECTORY;CONFIGURATION" # one value args
                        "CONDITION;DEFAULT_ENABLE_IF;TEST_DEPENDS;DRIVER;DRIVER_ARGS;DEPENDS;TEST_ARGS;SOURCES;LIBRARIES" # multi-value args
                        ${ARGN})

  set(BUILD_TESTING "${BUILD_TESTING}")

  # set the default values for optional parameters
  if (NOT CURTEST_EXE_NAME)
    set(CURTEST_EXE_NAME ${TestName})
  endif()

  # Strip test_ prefix from name
  if ("${TestName}" MATCHES "^test_([^/]*)$")
    string (REGEX REPLACE "^test_([^/]*)$" "\\1" _FANCY "${TestName}")
  else()
    set(_FANCY ${TestName})
  endif()

  # try to auto-detect the name of the source file if SOURCES are not
  # explicitly specified.
  if (NOT CURTEST_SOURCES)
    set(CURTEST_SOURCES "")
    set(_SDir "${PROJECT_SOURCE_DIR}")
    foreach(CURTEST_CANDIDATE "${CURTEST_EXE_NAME}.cpp"
                              "${CURTEST_EXE_NAME}.cc"
                              "tests/${CURTEST_EXE_NAME}.cpp"
                              "tests/${CURTEST_EXE_NAME}.cc")
      if (EXISTS "${_SDir}/${CURTEST_CANDIDATE}")
        set(CURTEST_SOURCES "${_SDir}/${CURTEST_CANDIDATE}")
      endif()
    endforeach()
  endif()

  # the default working directory is the content of
  # OPM_TEST_DEFAULT_WORKING_DIRECTORY or the source directory if this
  # is unspecified
  if (NOT CURTEST_WORKING_DIRECTORY)
    if (OPM_TEST_DEFAULT_WORKING_DIRECTORY)
      set(CURTEST_WORKING_DIRECTORY ${OPM_TEST_DEFAULT_WORKING_DIRECTORY})
    else()
      set(CURTEST_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()
  endif()

  # don't build the tests by _default_ if BUILD_TESTING is false,
  # i.e., when typing 'make' the tests are not build in that
  # case. They can still be build using 'make test-suite' and they can
  # be build and run using 'make check'
  set(CURTEST_EXCLUDE_FROM_ALL "")
  if (NOT "AND OR ${CURTEST_DEFAULT_ENABLE_IF}" STREQUAL "AND OR ")
    if (NOT ${CURTEST_DEFAULT_ENABLE_IF})
      set(CURTEST_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
    endif()
  endif()
  if (NOT BUILD_TESTING AND NOT CURTEST_ALWAYS_ENABLE)
    set(CURTEST_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
  endif()

  # figure out the test driver script and its arguments. (the variable
  # for the driver script may be empty. In this case the binary is run
  # "bare metal".)
  if (NOT CURTEST_DRIVER)
    set(CURTEST_DRIVER "${OPM_TEST_DRIVER}")
  endif()
  if (NOT CURTEST_DRIVER_ARGS)
    set(CURTEST_DRIVER_ARGS "${OPM_TEST_DRIVER_ARGS}")
  endif()

  # the libraries to link against
  if (NOT CURTEST_LIBRARIES)
    SET(CURTEST_LIBRARIES "${${project}_LIBRARIES}")
  endif()

  # determine if the test should be completely ignored, i.e., the
  # CONDITION argument evaluates to false. the "AND OR " is a hack
  # which is required to prevent CMake from evaluating the condition
  # in the string. (which might evaluate to an empty string even
  # though "${CURTEST_CONDITION}" is not empty.)
  if ("AND OR ${CURTEST_CONDITION}" STREQUAL "AND OR ")
    set(SKIP_CUR_TEST "0")
  elseif(${CURTEST_CONDITION})
    set(SKIP_CUR_TEST "0")
  else()
    set(SKIP_CUR_TEST "1")
  endif()

  if (NOT SKIP_CUR_TEST)
    if (CURTEST_ONLY_COMPILE)
      # only compile the binary but do not run it as a test
      add_executable("${CURTEST_EXE_NAME}" ${CURTEST_EXCLUDE_FROM_ALL} ${CURTEST_SOURCES})
      target_link_libraries (${CURTEST_EXE_NAME} ${CURTEST_LIBRARIES})
      get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
      add_static_analysis_tests(CURTEST_SOURCES dirs)

      if(TARGET ${project}_prepare)
        add_dependencies("${CURTEST_EXE_NAME}" ${project}_prepare)
      endif()
      if(CURTEST_DEPENDS)
        add_dependencies("${CURTEST_EXE_NAME}" ${CURTEST_DEPENDS})
      endif()
    else()
      if (NOT CURTEST_NO_COMPILE)
        # in addition to being run, the test must be compiled. (the
        # run-only case occurs if the binary is already compiled by an
        # earlier test.)
        add_executable("${CURTEST_EXE_NAME}" ${CURTEST_EXCLUDE_FROM_ALL} ${CURTEST_SOURCES})
        if(HAVE_DYNAMIC_BOOST_TEST)
          set_target_properties (${CURTEST_EXE_NAME} PROPERTIES
                                 COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK)
        endif()
        target_link_libraries (${CURTEST_EXE_NAME} ${CURTEST_LIBRARIES})
        get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
        add_static_analysis_tests(CURTEST_SOURCES dirs)

        if(CURTEST_DEPENDS)
          add_dependencies("${CURTEST_EXE_NAME}" ${CURTEST_DEPENDS})
        endif()
        if(TARGET ${project}_prepare)
          add_dependencies("${CURTEST_EXE_NAME}" ${project}_prepare)
        endif()
      endif()

      # figure out how the test should be run. if a test driver script
      # has been specified to supervise the test binary, use it else
      # run the test binary "naked".
      if (CURTEST_DRIVER)
        set(CURTEST_COMMAND ${CURTEST_DRIVER} ${CURTEST_DRIVER_ARGS} -e ${CURTEST_EXE_NAME} -- ${CURTEST_TEST_ARGS})
      else()
        set(CURTEST_COMMAND ${PROJECT_BINARY_DIR}/bin/${CURTEST_EXE_NAME})
        if (CURTEST_TEST_ARGS)
          list(APPEND CURTEST_COMMAND ${CURTEST_TEST_ARGS})
        endif()
      endif()

      add_test(NAME ${_FANCY}
               WORKING_DIRECTORY "${CURTEST_WORKING_DIRECTORY}"
               COMMAND ${CURTEST_COMMAND}
               CONFIGURATIONS ${CURTEST_CONFIGURATION})

      # specify the dependencies between the tests
      if (CURTEST_TEST_DEPENDS)
        set_tests_properties(${_FANCY} PROPERTIES DEPENDS "${CURTEST_TEST_DEPENDS}")
      endif()

      # tell ctest how many cores it should reserve to run the test
      if (CURTEST_PROCESSORS)
        set_tests_properties(${_FANCY} PROPERTIES PROCESSORS "${CURTEST_PROCESSORS}")
      endif()
    endif()

    if (NOT CURTEST_NO_COMPILE)
      if(NOT TARGET test-suite)
        add_custom_target(test-suite)
      endif()
      add_dependencies(test-suite "${CURTEST_EXE_NAME}")
    endif()
  endif()
endmacro()

# macro to set the default test driver script and the its default
# arguments
macro(opm_set_test_driver DriverBinary DriverDefaultArgs)
  set(OPM_TEST_DRIVER "${DriverBinary}")
  set(OPM_TEST_DRIVER_ARGS "${DriverDefaultArgs}")
endmacro()

# macro to set the default test driver script and the its default
# arguments
macro(opm_set_test_default_working_directory Dir)
  set(OPM_TEST_DEFAULT_WORKING_DIRECTORY "${Dir}")
endmacro()
