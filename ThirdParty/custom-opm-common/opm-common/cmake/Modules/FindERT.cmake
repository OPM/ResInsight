# - Find the Ensemble-based Reservoir Tool (ERT)
#
# Set the cache variable ERT_ROOT to the install location of the ERT
# libraries and header files.
#
# If found, it sets these variables:
#
#	ERT_INCLUDE_DIRS      Header file directories
#	ERT_LIBRARIES         Archives and shared objects
#	ERT_CONFIG_VARS       Definitions that goes in config.h
#	ERT_LINKER_FLAGS      Options that must be passed to linker
#
# It will also add to CMAKE_C_FLAGS and CMAKE_CXX_FLAGS if necessary to
# link with the ERT libraries.

# variables to pass on to other packages
if (FIND_QUIETLY)
  set (ERT_QUIET "QUIET")
else (FIND_QUIETLY)
  set (ERT_QUIET "")
endif (FIND_QUIETLY)

# if a directory has been specified by the user, then don't go look
# in the system directories as well
if (ERT_ROOT)
  set (_no_default_path "NO_DEFAULT_PATH")
else (ERT_ROOT)
  set (_no_default_path "")
endif (ERT_ROOT)

# ERT doesn't have any config-mode file, so we need to specify the root
# directory in its own variable
find_path (ERT_ECL_INCLUDE_DIR
  NAMES "ert/ecl/ecl_util.h"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}/../ert"
  PATH_SUFFIXES "devel/libecl/include/" "include"
  DOC "Path to ERT Eclipse library header files"
  ${_no_default_path}
  )
find_path (ERT_ECL_WELL_INCLUDE_DIR
  NAMES "ert/ecl_well/well_const.h"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}/../ert"
  PATH_SUFFIXES "devel/libecl_well/include/" "include"
  DOC "Path to ERT Eclipse library header files"
  ${_no_default_path}
  )
find_path (ERT_ECLXX_INCLUDE_DIR
  NAMES "ert/ecl/EclKW.hpp"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}/../ert"
  PATH_SUFFIXES "devel/libeclxx/include/" "include"
  DOC "Path to ERT Eclipse C++ library header files"
  ${_no_default_path}
  )
find_path (ERT_UTIL_INCLUDE_DIR
  NAMES "ert/util/stringlist.h"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}/../ert"
  PATH_SUFFIXES "devel/libert_util/include/" "include"
  DOC "Path to ERT Eclipse library header files"
  ${_no_default_path}
  )
find_path (ERT_UTILXX_INCLUDE_DIR
  NAMES "ert/util/ert_unique_ptr.hpp"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}/../ert"
  PATH_SUFFIXES "devel/libert_utilxx/include/" "include"
  DOC "Path to ERT Eclipse C++ library header files"
  ${_no_default_path}
  )  
find_path (ERT_GEN_INCLUDE_DIR
  NAMES "ert/util/int_vector.h"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_SOURCE_DIR}/../ert"
  PATH_SUFFIXES "devel/libert_util/include"
                "include" "build/libert_util/include"
  DOC "Path to ERT generated library header files"
  ${_no_default_path}
  )


# need all of these libraries
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)
find_library (ERT_LIBRARY_ECL
  NAMES "ecl"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_BINARY_DIR}/../ert"
        "${PROJECT_SOURCE_DIR}/../ert/build"
        "${PROJECT_SOURCE_DIR}/../ert/devel/build"
        "${PROJECT_BINARY_DIR}/../ert-build"
        "${PROJECT_BINARY_DIR}/../ert/devel"
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  DOC "Path to ERT Eclipse library archive/shared object files"
  ${_no_default_path}
  )
find_library (ERT_LIBRARY_ECLXX
  NAMES "eclxx"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_BINARY_DIR}/../ert"
        "${PROJECT_SOURCE_DIR}/../ert/build"
        "${PROJECT_SOURCE_DIR}/../ert/devel/build"
        "${PROJECT_BINARY_DIR}/../ert-build"
        "${PROJECT_BINARY_DIR}/../ert/devel"
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  DOC "Path to ERT Eclipse C++ library archive/shared object files"
  ${_no_default_path}
  )
find_library (ERT_LIBRARY_ECL_WELL
  NAMES "ecl_well"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_BINARY_DIR}/../ert"
        "${PROJECT_SOURCE_DIR}/../ert/build"
        "${PROJECT_SOURCE_DIR}/../ert/devel/build"
        "${PROJECT_BINARY_DIR}/../ert-build"
        "${PROJECT_BINARY_DIR}/../ert/devel"
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  DOC "Path to ERT Eclipse library archive/shared object files"
  ${_no_default_path}
  )
find_library (ERT_LIBRARY_GEOMETRY
  NAMES "ert_geometry"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_BINARY_DIR}/../ert"
        "${PROJECT_SOURCE_DIR}/../ert/build"
        "${PROJECT_SOURCE_DIR}/../ert/devel/build"
        "${PROJECT_BINARY_DIR}/../ert-build"
        "${PROJECT_BINARY_DIR}/../ert/devel"
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  DOC "Path to ERT Geometry library archive/shared object files"
  ${_no_default_path}
  )
find_library (ERT_LIBRARY_UTIL
  NAMES "ert_util"
  HINTS "${ERT_ROOT}"
  PATHS "${PROJECT_BINARY_DIR}/../ert"
        "${PROJECT_SOURCE_DIR}/../ert/build"
        "${PROJECT_SOURCE_DIR}/../ert/devel/build"
        "${PROJECT_BINARY_DIR}/../ert-build"
        "${PROJECT_BINARY_DIR}/../ert/devel"
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
  DOC "Path to ERT Utilities library archive/shared object files"
  ${_no_default_path}
  )
# the "library" found here is actually a list of several files
list (APPEND ERT_INCLUDE_DIR
  ${ERT_ECL_INCLUDE_DIR}
  ${ERT_ECL_WELL_INCLUDE_DIR}
  ${ERT_ECLXX_INCLUDE_DIR}
  ${ERT_UTIL_INCLUDE_DIR}
  ${ERT_UTILXX_INCLUDE_DIR}
  ${ERT_GEN_INCLUDE_DIR}
  )
list (APPEND ERT_LIBRARY
  ${ERT_LIBRARY_ECL}
  ${ERT_LIBRARY_ECLXX}
  ${ERT_LIBRARY_ECL_WELL}
  ${ERT_LIBRARY_GEOMETRY}
  ${ERT_LIBRARY_UTIL}
  )
list (APPEND ERT_LIBRARIES ${ERT_LIBRARY})
list (APPEND ERT_INCLUDE_DIRS ${ERT_INCLUDE_DIR})

# if we didn't find any files, then don't proceed through the entire dependency list
include (FindPackageHandleStandardArgs)
if (ERT_INCLUDE_DIR MATCHES "-NOTFOUND" OR ERT_LIBRARIES MATCHES "-NOTFOUND")
  find_package_handle_standard_args (ERT
	DEFAULT_MSG
	ERT_INCLUDE_DIR ERT_LIBRARY
	)
  # clear the cache so the find probe is attempted again if files becomes
  # available (only upon a unsuccessful *compile* should we disable further
  # probing)
  set (HAVE_ERT)
  unset (HAVE_ERT CACHE)
  return ()
endif (ERT_INCLUDE_DIR MATCHES "-NOTFOUND" OR ERT_LIBRARIES MATCHES "-NOTFOUND")


# dependencies

# parallel programming
include (UseOpenMP)
find_openmp (ERT)

# compression library
find_package (ZLIB ${ERT_QUIET})
if (ZLIB_FOUND)
  list (APPEND ERT_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
  list (APPEND ERT_LIBRARIES ${ZLIB_LIBRARIES})
endif (ZLIB_FOUND)

# numerics
find_package (BLAS ${ERT_QUIET})
if (BLAS_FOUND)
  list (APPEND ERT_INCLUDE_DIRS ${BLAS_INCLUDE_DIRS})
  list (APPEND ERT_LIBRARIES ${BLAS_LIBRARIES})
  list (APPEND ERT_LINKER_FLAGS ${BLAS_LINKER_FLAGS})
endif (BLAS_FOUND)

find_package (LAPACK ${ERT_QUIET})
if (LAPACK_FOUND)
  list (APPEND ERT_INCLUDE_DIRS ${LAPACK_INCLUDE_DIRS})
  list (APPEND ERT_LIBRARIES ${LAPACK_LIBRARIES})
  list (APPEND ERT_LINKER_FLAGS ${LAPACK_LINKER_FLAGS})
endif (LAPACK_FOUND)

# math library (should exist on all unices; automatically linked on Windows)
if (UNIX)
  find_library (MATH_LIBRARY
	NAMES "m"
	)
  list (APPEND ERT_LIBRARIES ${MATH_LIBRARY})
endif (UNIX)

# if shared libraries are disabled on linux, explcitly linking to the
# pthreads library is required by ERT
find_package(Threads ${ERT_QUIET})
if (CMAKE_THREAD_LIBS_INIT)
  list (APPEND ERT_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

# Platform specific library where dlopen with friends lives
list (APPEND ERT_LIBRARIES ${CMAKE_DL_LIBS})

# since OpenMP often implies pthreads, we need to tidy up
# (last instance of library must be left standing, thus reversing that
# list before removing duplicates)
include (Duplicates)
remove_dup_deps (ERT)

# see if we can compile a minimum example
# CMake logical test doesn't handle lists (sic)
if (NOT (ERT_INCLUDE_DIR MATCHES "-NOTFOUND" OR ERT_LIBRARIES MATCHES "-NOTFOUND"))
  include (CMakePushCheckState)
  include (CheckCSourceCompiles)
  cmake_push_check_state ()
  set (CMAKE_REQUIRED_INCLUDES ${ERT_INCLUDE_DIR})
  set (CMAKE_REQUIRED_LIBRARIES ${ERT_LIBRARIES})
  check_cxx_source_compiles (
"#include <ert/ecl/EclKW.hpp>
int main ( ) {
  ERT::EclKW<int> kw(\"SATNUM\" , 1000);
  kw[0] = 10;
  return 0;
}" HAVE_ERT)
  cmake_pop_check_state ()
else (NOT (ERT_INCLUDE_DIR MATCHES "-NOTFOUND" OR ERT_LIBRARIES MATCHES "-NOTFOUND"))
  # clear the cache so the find probe is attempted again if files becomes
  # available (only upon a unsuccessful *compile* should we disable further
  # probing)
  set (HAVE_ERT)
  unset (HAVE_ERT CACHE)
endif (NOT (ERT_INCLUDE_DIR MATCHES "-NOTFOUND" OR ERT_LIBRARIES MATCHES "-NOTFOUND"))

# if the test program didn't compile, but was required to do so, bail
# out now and display an error; otherwise limp on
find_package_handle_standard_args (ERT
  DEFAULT_MSG
  ERT_INCLUDE_DIR ERT_LIBRARY HAVE_ERT
  )




