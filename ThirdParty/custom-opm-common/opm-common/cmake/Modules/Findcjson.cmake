# Look for the cjson library; will probably newer be found.
# If found, it sets these variables:
#
#       CJSON_INCLUDE_DIRS      Header file directories
#       CJSON_LIBRARIES         Archive/shared objects

include (FindPackageHandleStandardArgs)

if ((NOT CJSON_ROOT) AND OPM_PARSER_ROOT)
  set( CJSON_ROOT ${OPM_PARSER_ROOT}) 
endif()

if (CJSON_ROOT)
  set (_no_default_path "NO_DEFAULT_PATH")
else (CJSON_ROOT)
  set (_no_default_path "")
endif (CJSON_ROOT)


find_path (CJSON_INCLUDE_DIR
  NAMES "cjson/cJSON.h"
  HINTS "${CJSON_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/../opm-parser"
  PATH_SUFFIXES "include" "opm/json"
  DOC "Path to cjson library header files"
  ${_no_default_path} )

# find out the size of a pointer. this is required to only search for
# libraries in the directories relevant for the architecture
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)

string(REGEX REPLACE "${PROJECT_SOURCE_DIR}/?(.*)" "\\1"  BUILD_DIR_SUFFIX "${PROJECT_BINARY_DIR}")

find_library (CJSON_LIBRARY
  NAMES "cjson"
  HINTS "${CJSON_ROOT}"
  PATHS "${PROJECT_BINARY_DIR}/../opm-parser"
        "${PROJECT_BINARY_DIR}/../opm-parser${BUILD_DIR_SUFFIX}"
        "${PROJECT_BINARY_DIR}/../../opm-parser/${BUILD_DIR_SUFFIX}"
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
                "opm/json"
  DOC "Path to cjson library archive/shared object files"
  ${_no_default_path} )

# setup list of all required libraries to link with cjson
set (CJSON_INCLUDE_DIRS ${CJSON_INCLUDE_DIR})
set (CJSON_LIBRARIES ${CJSON_LIBRARY})

# math library (should exist on all unices; automatically linked on Windows)
if (UNIX)
  find_library (MATH_LIBRARY NAMES "m")
  list (APPEND CJSON_LIBRARIES ${MATH_LIBRARY})
endif (UNIX)

# see if we can compile a minimum example
# CMake logical test doesn't handle lists (sic)
if (NOT (CJSON_INCLUDE_DIRS MATCHES "-NOTFOUND" OR CJSON_LIBRARIES MATCHES "-NOTFOUND"))
  include (CMakePushCheckState)
  include (CheckCSourceCompiles)
  cmake_push_check_state ()
  set (CMAKE_REQUIRED_INCLUDES ${CJSON_INCLUDE_DIRS})
  set (CMAKE_REQUIRED_LIBRARIES ${CJSON_LIBRARIES})

  check_c_source_compiles (
"#include <stdlib.h>
#include <cjson/cJSON.h>
int main (void) {
    cJSON root;
  return 0;
}"  HAVE_CJSON)
  cmake_pop_check_state ()
else ()
  # clear the cache so the find probe is attempted again if files becomes
  # available (only upon a unsuccessful *compile* should we disable further
  # probing)
  set (HAVE_CJSON)
  unset (HAVE_CJSON CACHE)
endif ()

# if the test program didn't compile, but was required to do so, bail
# out now and display an error; otherwise limp on
set (CJSON_FIND_REQUIRED ${cjson_FIND_REQUIRED})
set (CJSON_FIND_QUIETLY ${cjson_FIND_QUIETLY})
find_package_handle_standard_args (CJSON
  DEFAULT_MSG
  CJSON_INCLUDE_DIRS CJSON_LIBRARIES HAVE_CJSON
  )
set (cjson_FOUND ${CJSON_FOUND})
