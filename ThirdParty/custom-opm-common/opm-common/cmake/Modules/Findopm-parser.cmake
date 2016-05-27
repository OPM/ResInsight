# Find the OPM Eclipse input parser.
#
# Set the cache variable OPM_PARSER_ROOT to the install location of the
# library, or OPM_ROOT to the parent directory of the build tree.
#
# If found, it sets these variables:
#
#       HAVE_OPM_PARSER              Defined if a test program compiled
#       OPM_PARSER_INCLUDE_DIRS      Header file directories
#       OPM_PARSER_LIBRARIES         Archives and shared objects

include (FindPackageHandleStandardArgs)

# variables to pass on to other packages
if (FIND_QUIETLY)
  set (OPM_PARSER_QUIET "QUIET")
else ()
  set (OPM_PARSER_QUIET "")
endif ()

# use lowercase versions of the variables if those are set
if (opm-parser_ROOT)
  set (OPM_PARSER_ROOT ${opm-parser_ROOT})
endif ()
if (opm_ROOT)
  set (OPM_ROOT ${opm_ROOT})
endif ()

# inherit "suite" root if not specifically set for this library
if ((NOT OPM_PARSER_ROOT) AND OPM_ROOT)
  set (OPM_PARSER_ROOT "${OPM_ROOT}/opm-parser")
endif ()

# Detect the build dir suffix or subdirectory
string(REGEX REPLACE "${PROJECT_SOURCE_DIR}/?(.*)" "\\1"  BUILD_DIR_SUFFIX "${PROJECT_BINARY_DIR}")

# if a root is specified, then don't search in system directories
# or in relative directories to this one
if (OPM_PARSER_ROOT)
  set (_no_default_path "NO_DEFAULT_PATH")
  set (_opm_parser_source "")
  set (_opm_parser_build "")
else ()
  set (_no_default_path "")
  set (_opm_parser_source
    "${PROJECT_SOURCE_DIR}/../opm-parser")
  set (_opm_parser_build
    "${PROJECT_BINARY_DIR}/../opm-parser"
    "${PROJECT_BINARY_DIR}/../opm-parser${BUILD_DIR_SUFFIX}"
    "${PROJECT_BINARY_DIR}/../../opm-parser/${BUILD_DIR_SUFFIX}")
endif ()

# use this header as signature
find_path (OPM_PARSER_INCLUDE_DIR
  NAMES "opm/parser/eclipse/Parser/Parser.hpp"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_source}
  PATH_SUFFIXES "include"
  DOC "Path to OPM parser header files"
  ${_no_default_path} )

find_path (OPM_PARSER_GEN_INCLUDE_DIR
  NAMES "opm/parser/eclipse/Parser/ParserKeywords.hpp"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_build}
  PATH_SUFFIXES "generated-source/include" "include"
  DOC "Path to OPM parser generated header files"
  ${_no_default_path} )


# backup: if we didn't find any headers there, but a CMakeCache.txt,
# then it is probably a build directory; read the CMake cache of
# opm-parser to figure out where the source directory is
if ((NOT OPM_PARSER_INCLUDE_DIR) AND
        (OPM_PARSER_ROOT AND (EXISTS "${OPM_PARSER_ROOT}/CMakeCache.txt")))
  set (_regex "^OPMParser_SOURCE_DIR:STATIC=\(.*\)$")
  file (STRINGS
        "${OPM_PARSER_ROOT}/CMakeCache.txt"
        _cache_entry
        REGEX "${_regex}")
  string(REGEX REPLACE "${_regex}" "\\1"
        OPM_PARSER_INCLUDE_DIR
        "${_cache_entry}")
  if (OPM_PARSER_INCLUDE_DIR)
        set (OPM_PARSER_INCLUDE_DIR "${OPM_PARSER_INCLUDE_DIR}"
          CACHE PATH "Path to OPM parser header files" FORCE)
  endif ()
endif ()

# find out the size of a pointer. this is required to only search for
# libraries in the directories relevant for the architecture
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif ()

# these libraries constitute the parser core
find_library (OPM_PARSER_LIBRARY
  NAMES "opmparser"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_build}
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
                "opm/parser/eclipse"
  DOC "Path to OPM parser library archive/shared object files"
  ${_no_default_path} )

# find the OPM-parser wrapper library around cJSON
find_library (OPM_JSON_LIBRARY
  NAMES "opmjson"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_build}
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
                "opm/json"
  DOC "Path to OPM JSON library archive/shared object files"
  ${_no_default_path} )

# get the prerequisite ERT libraries
if (NOT ERT_FOUND)
  find_package(ERT ${OPM_PARSER_QUIET})
endif ()

# get the prerequisite Boost libraries
find_package(Boost 1.44.0 COMPONENTS filesystem date_time system unit_test_framework regex ${OPM_PARSER_QUIET})

if (ERT_FOUND AND Boost_FOUND AND
    OPM_PARSER_LIBRARY AND OPM_JSON_LIBRARY AND OPM_PARSER_INCLUDE_DIR)
  # setup list of all required libraries to link with opm-parser. notice that
  # we use the plural form to get *all* the libraries needed by cjson
  set (opm-parser_INCLUDE_DIRS
    ${OPM_PARSER_INCLUDE_DIR}
    ${OPM_PARSER_GEN_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${ERT_INCLUDE_DIRS})

  set (opm-parser_LIBRARIES
    ${OPM_PARSER_LIBRARY}
    ${OPM_JSON_LIBRARY}
    ${Boost_LIBRARIES}
    ${ERT_LIBRARIES})

  # see if we can compile a minimum example
  # CMake logical test doesn't handle lists (sic)
  include (CMakePushCheckState)
  include (CheckCSourceCompiles)
  cmake_push_check_state ()
  set (CMAKE_REQUIRED_INCLUDES ${opm-parser_INCLUDE_DIRS})
  set (CMAKE_REQUIRED_LIBRARIES ${opm-parser_LIBRARIES})

  check_cxx_source_compiles (
      "#include <cstdlib>
#include <opm/parser/eclipse/Deck/Deck.hpp>

int main (void) {
   return EXIT_SUCCESS;
}" HAVE_OPM_PARSER)
  cmake_pop_check_state ()
endif()

# if the test program didn't compile, but was required to do so, bail
# out now and display an error; otherwise limp on
set (OPM_PARSER_FIND_REQUIRED ${opm-parser_FIND_REQUIRED})
find_package_handle_standard_args (opm-parser
  DEFAULT_MSG
  opm-parser_INCLUDE_DIRS opm-parser_LIBRARIES HAVE_OPM_PARSER
    )

set (opm-parser_CONFIG_VARS "HAVE_OPM_PARSER;HAVE_REGEX")
set (opm-parser_FOUND ${OPM-PARSER_FOUND})

mark_as_advanced(opm-parser_LIBRARIES opm-parser_INCLUDE_DIRS OPM-PARSER_FOUND)
