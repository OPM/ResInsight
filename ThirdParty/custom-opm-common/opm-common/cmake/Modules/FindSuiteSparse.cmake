# - Find Tim Davis' SuiteSparse collection of sparse matrix libraries
#
# Synopsis:
#   find_package (SuiteSparse COMPONENTS <list-of-components>)
#
# Components are:
#   amd              Approximate Minimum Degree ordering
#   camd             Constrained Approximate Minimum Degree ordering
#   colamd           COLumn Approximate Minimum Degree ordering
#   ccolamd          Constrained COLumn Approximate Minimum Degree ordering
#   cholmod          Supernodal sparse Cholesky factorization and update
#   umfpack          Unsymmetric MultiFrontal sparse LU factorization
#
# The following variables will be set:
#
#   SuiteSparse_FOUND            True if all dependencies are satisfied
#   SuiteSparse_Xxx_FOUND        True if module Xxx is found
#   HAVE_SUITESPARSE_Xxx_H       Binary value indicating presence of header
#   SuiteSparse_INCLUDE_DIRS     Paths containing the SuiteSparse header files
#   SuiteSparse_LIBRARIES        Name of the libraries which must be linked
#   SuiteSparse_DEFINITIONS      Defines that must be passed to the compiler
#   SuiteSparse_LINKER_FLAGS     Options that must be passed when linking
#
# The following options can be set to configure the module:
#
#   SUITESPARSE_USE_STATIC       Link with a static library, even if a
#                                dynamic library is also present. Note that
#                                setting this to OFF does not ensure that a
#                                shared library will be used.
#
# See <http://www.cise.ufl.edu/research/sparse/SuiteSparse>.

# Copyright (C) 2012 Uni Research AS
# This file is licensed under the GNU General Public License v3.0

function (try_compile_umfpack varname)
  include (CMakePushCheckState)
  include (CheckCSourceCompiles)
  cmake_push_check_state ()
  set (CMAKE_REQUIRED_INCLUDES ${UMFPACK_INCLUDE_DIRS})
  set (CMAKE_REQUIRED_LIBRARIES ${UMFPACK_LIBRARY} ${ARGN} ${SuiteSparse_EXTRA_LIBS})
  check_c_source_compiles (
        "#include <umfpack.h>
int main (void) {
  void *Symbolic, *Numeric;
  double Info[UMFPACK_INFO], Control[UMFPACK_CONTROL];
  umfpack_dl_defaults(Control);
  umfpack_dl_symbolic(0, 0, 0, 0, 0,
                      &Symbolic, Control, Info);
  umfpack_dl_numeric (0, 0, 0,
                      Symbolic, &Numeric, Control, Info);
  umfpack_dl_free_symbolic(&Symbolic);
  umfpack_dl_solve(UMFPACK_A, 0, 0, 0, 0, 0,
                   Numeric, Control, Info);
  umfpack_dl_free_numeric(&Numeric);
  umfpack_timer ();
  return 0;
}" ${varname})
  cmake_pop_check_state ()
  set (${varname} "${${varname}}" PARENT_SCOPE)
endfunction (try_compile_umfpack varname)

# variables to pass on to other packages
if (FIND_QUIETLY)
  set (SuiteSparse_QUIET "QUIET")
else (FIND_QUIETLY)
  set (SuiteSparse_QUIET "")
endif (FIND_QUIETLY)

# we need to link to BLAS and LAPACK
if (NOT BLAS_FOUND)
  find_package (BLAS ${SuiteSparse_QUIET} REQUIRED)
endif (NOT BLAS_FOUND)
if (NOT LAPACK_FOUND)
  find_package (LAPACK ${SuiteSparse_QUIET} REQUIRED)
endif (NOT LAPACK_FOUND)

# we also need the math part of the runtime library
find_library (MATH_LIBRARY NAMES "m")
set (SuiteSparse_EXTRA_LIBS ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES} ${MATH_LIBRARY})

# if we don't get any further clues about where to look, then start
# roaming around the system
set (_no_default_path "")

# search system directories by default
set (SuiteSparse_SEARCH_PATH)

# pick up paths from the environment if specified there; these replace the
# pre-defined paths so that we don't accidentially pick up old stuff
if (NOT $ENV{SuiteSparse_DIR} STREQUAL "")
  set (SuiteSparse_SEARCH_PATH "$ENV{SuiteSparse_DIR}")
endif (NOT $ENV{SuiteSparse_DIR} STREQUAL "")
if (SuiteSparse_DIR)
  set (SuiteSparse_SEARCH_PATH "${SuiteSparse_DIR}")
endif (SuiteSparse_DIR)
# CMake uses _DIR suffix as default for config-mode files; it is unlikely
# that we are building SuiteSparse ourselves; use _ROOT suffix to specify
# location to pre-canned binaries
if (NOT $ENV{SuiteSparse_ROOT} STREQUAL "")
  set (SuiteSparse_SEARCH_PATH "$ENV{SuiteSparse_ROOT}")
endif (NOT $ENV{SuiteSparse_ROOT} STREQUAL "")
if (SuiteSparse_ROOT)
  set (SuiteSparse_SEARCH_PATH "${SuiteSparse_ROOT}")
endif (SuiteSparse_ROOT)
# most commonly, we use the uppercase version of this variable
if (SUITESPARSE_ROOT)
  set (SuiteSparse_SEARCH_PATH "${SUITESPARSE_ROOT}")
endif (SUITESPARSE_ROOT)

# if we have specified a search path, then confine ourselves to that
if (SuiteSparse_SEARCH_PATH)
  set (_no_default_path "NO_DEFAULT_PATH")
endif (SuiteSparse_SEARCH_PATH)

# transitive closure of dependencies; after this SuiteSparse_MODULES is the
# full list of modules that must be found to satisfy the user's link demands
set (SuiteSparse_MODULES ${SuiteSparse_FIND_COMPONENTS})
list (FIND SuiteSparse_MODULES "umfpack" UMFPACK_DESIRED)
if (NOT UMFPACK_DESIRED EQUAL -1)
  list (APPEND SuiteSparse_MODULES amd cholmod)
endif (NOT UMFPACK_DESIRED EQUAL -1)
list (FIND SuiteSparse_MODULES "cholmod" CHOLMOD_DESIRED)
if (NOT CHOLMOD_DESIRED EQUAL -1)
  list (APPEND SuiteSparse_MODULES amd camd colamd)
endif (NOT CHOLMOD_DESIRED EQUAL -1)
if (SuiteSparse_MODULES)
  list (REMOVE_DUPLICATES SuiteSparse_MODULES)
endif (SuiteSparse_MODULES)

# if someone else already have found all the packages for us, then don't do anything
set (SuiteSparse_EVERYTHING_FOUND TRUE)
foreach (module IN LISTS SuiteSparse_MODULES)
  string (TOUPPER ${module} MODULE)
  if (NOT SuiteSparse_${MODULE}_FOUND)
	set (SuiteSparse_EVERYTHING_FOUND FALSE)
	break ()
  endif (NOT SuiteSparse_${MODULE}_FOUND)
endforeach (module)
if (SuiteSparse_EVERYTHING_FOUND)
  return ()
endif (SuiteSparse_EVERYTHING_FOUND)

# only search in architecture-relevant directory
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)

# if we are told to link SuiteSparse statically, add these parts
# to the name so we always match only that particular type of lib
option (SUITESPARSE_USE_STATIC "Link SuiteSparse statically" OFF)
mark_as_advanced (SUITESPARSE_USE_STATIC)
if (SUITESPARSE_USE_STATIC)
  set (_pref_ "${CMAKE_STATIC_LIBRARY_PREFIX}")
  set (_suff_ "${CMAKE_STATIC_LIBRARY_SUFFIX}")
else (SUITESPARSE_USE_STATIC)
  set (_pref_ "")
  set (_suff_ "")
endif (SUITESPARSE_USE_STATIC)

# if SuiteSparse >= 4.0 we must also link with libsuitesparseconfig
# assume that this is the case if we find the library; otherwise just
# ignore it (older versions don't have a file named like this)
find_library (config_LIBRARY
  NAMES "${_pref_}suitesparseconfig${_suff_}"
  PATHS ${SuiteSparse_SEARCH_PATH}
  PATH_SUFFIXES ".libs" "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}" "lib/ufsparse"
  ${_no_default_path}
  )
if (config_LIBRARY)
  list (APPEND SuiteSparse_EXTRA_LIBS ${config_LIBRARY})
  # POSIX.1-2001 REALTIME portion require us to link this library too for
  # clock_gettime() which is used by suitesparseconfig
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	list (APPEND SuiteSparse_EXTRA_LIBS "-lrt")
  endif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
endif (config_LIBRARY)

# search filesystem for each of the module individually
foreach (module IN LISTS SuiteSparse_MODULES)
  string (TOUPPER ${module} MODULE)
  # search for files which implements this module
  find_path (${MODULE}_INCLUDE_DIR
	NAMES ${module}.h
	PATHS ${SuiteSparse_SEARCH_PATH}
	PATH_SUFFIXES "include" "include/suitesparse" "include/ufsparse" "${MODULE}/Include"
	${_no_default_path}
	)

  find_library (${MODULE}_LIBRARY
	NAMES "${_pref_}${module}${_suff_}"
	PATHS ${SuiteSparse_SEARCH_PATH}
	PATH_SUFFIXES "lib/.libs" "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}" "lib/ufsparse" "${MODULE}/Lib"
	${_no_default_path}
	)
  # start out by including the module itself; other dependencies will be added later
  set (${MODULE}_INCLUDE_DIRS ${${MODULE}_INCLUDE_DIR})
  set (${MODULE}_LIBRARIES ${${MODULE}_LIBRARY})
endforeach (module)

# insert any inter-modular dependencies here
if (CHOLMOD_LIBRARY)
  list (APPEND CHOLMOD_LIBRARIES ${AMD_LIBRARIES} ${COLAMD_LIBRARIES})
  # optional libraries; don't insert any -NOT_FOUND paths
  if (CAMD_LIBRARY)
	list (APPEND CHOLMOD_LIBRARIES ${CAMD_LIBRARIES})
  endif (CAMD_LIBRARY)
  if (CCOLAMD_LIBRARY)
	list (APPEND CHOLMOD_LIBRARIES ${CCOLAMD_LIBRARIES})
  endif (CCOLAMD_LIBRARY)
  list (REVERSE CHOLMOD_LIBRARIES)
  # always remove the *first* library from the list
  list (REMOVE_DUPLICATES CHOLMOD_LIBRARIES)
  list (REVERSE CHOLMOD_LIBRARIES)
endif (CHOLMOD_LIBRARY)
if (UMFPACK_LIBRARY)
  set (UMFPACK_EXTRA_LIBS)
  # test if umfpack is usable with only amd and not cholmod
  try_compile_umfpack (HAVE_UMFPACK_WITHOUT_CHOLMOD ${AMD_LIBRARIES})
  if (HAVE_UMFPACK_WITHOUT_CHOLMOD)
	list (APPEND UMFPACK_EXTRA_LIBS ${AMD_LIBRARIES})
  else (HAVE_UMFPACK_WITHOUT_CHOLMOD)
	if (CHOLMOD_LIBRARIES)
	  try_compile_umfpack (HAVE_UMFPACK_WITH_CHOLMOD ${CHOLMOD_LIBRARIES})
	  if (HAVE_UMFPACK_WITH_CHOLMOD)
		list (APPEND UMFPACK_EXTRA_LIBS ${CHOLMOD_LIBRARIES})
	  else (HAVE_UMFPACK_WITH_CHOLMOD)
		set (UMFPACK_EXTRA_LIBS "-NOTFOUND")
	  endif (HAVE_UMFPACK_WITH_CHOLMOD)
	else (CHOLMOD_LIBRARIES)
	  # if we don't have cholmod, then we certainly cannot have umfpack with cholmod
	  set (UMFPACK_EXTRA_LIBS "-NOTFOUND")
	endif (CHOLMOD_LIBRARIES)
  endif (HAVE_UMFPACK_WITHOUT_CHOLMOD)
  list (APPEND UMFPACK_LIBRARIES ${UMFPACK_EXTRA_LIBS})
  list (REVERSE UMFPACK_LIBRARIES)
  list (REMOVE_DUPLICATES UMFPACK_LIBRARIES)
  list (REVERSE UMFPACK_LIBRARIES)
endif (UMFPACK_LIBRARY)

# don't reset these sets; if two packages request SuiteSparse with
# different modules, we want the sets to be merged
#set (SuiteSparse_LIBRARIES "")
#set (SuiteSparse_INCLUDE_DIRS "")

# determine which modules were found based on whether all dependencies
# were satisfied; create a list of ALL modules (specified) that was found
# (to be included in one swoop in CMakeLists.txt)
set (SuiteSparse_FOUND TRUE)
foreach (module IN LISTS SuiteSparse_FIND_COMPONENTS)
  string (TOUPPER ${module} MODULE)
  set (SuiteSparse_${MODULE}_FOUND TRUE)
  foreach (file IN LISTS ${MODULE}_INCLUDE_DIRS ${MODULE}_LIBRARIES)
	if (NOT EXISTS ${file})
	  set (SuiteSparse_${MODULE}_FOUND FALSE)
	endif (NOT EXISTS ${file})
  endforeach (file)
  if (NOT SuiteSparse_${MODULE}_FOUND)
	set (SuiteSparse_FOUND FALSE)
	# use empty string instead of zero, so it can be tested with #ifdef
	# as well as #if in the source code
	set (HAVE_SUITESPARSE_${MODULE}_H "" CACHE INT "Is ${module} header present?")
  else (NOT SuiteSparse_${MODULE}_FOUND)
	set (HAVE_SUITESPARSE_${MODULE}_H 1 CACHE INT "Is ${module} header present?")
	list (APPEND SuiteSparse_LIBRARIES "${${MODULE}_LIBRARIES}")
	list (APPEND SuiteSparse_LINKER_FLAGS "${${MODULE}_LINKER_FLAGS}")
	list (APPEND SuiteSparse_INCLUDE_DIRS "${${MODULE}_INCLUDE_DIRS}")
  endif (NOT SuiteSparse_${MODULE}_FOUND)
  mark_as_advanced (HAVE_SUITESPARSE_${MODULE}_H)
  mark_as_advanced (${MODULE}_INCLUDE_DIR)
  mark_as_advanced (${MODULE}_LIBRARY)
endforeach (module)

if (SuiteSparse_INCLUDE_DIRS)
  list (REMOVE_DUPLICATES SuiteSparse_INCLUDE_DIRS)
endif (SuiteSparse_INCLUDE_DIRS)
if (SuiteSparse_LIBRARIES)
  list (REVERSE SuiteSparse_LIBRARIES)
  list (REMOVE_DUPLICATES SuiteSparse_LIBRARIES)
  list (REVERSE SuiteSparse_LIBRARIES)
endif (SuiteSparse_LIBRARIES)

# print a message to indicate status of this package
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (SuiteSparse
  DEFAULT_MSG
  SuiteSparse_LIBRARIES
  SuiteSparse_INCLUDE_DIRS
  )

# add these after checking to not pollute the message output (checking for
# BLAS and LAPACK is REQUIRED so if they are not found, we'll have failed
# already; suitesparseconfig is "optional" anyway)
list (APPEND SuiteSparse_LIBRARIES ${SuiteSparse_EXTRA_LIBS})
