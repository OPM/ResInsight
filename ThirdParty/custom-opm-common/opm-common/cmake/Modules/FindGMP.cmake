# - Find the GNU Multiprecision library.
#
# Will define the following imported target for usage:
# - GMP::gmp Target for linking/compiling with C library
# - GMP::gmpxx Target for linking/compiling with C++ library
find_path(GMP_INCLUDE_DIR gmp.h)
find_library(GMP_LIBRARY gmp)
find_path(GMPXX_INCLUDE_DIR gmpxx.h)
find_library(GMPXX_LIBRARY gmpxx)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
  DEFAULT_MSG
  GMPXX_LIBRARY GMPXX_INCLUDE_DIR GMP_INCLUDE_DIR GMP_LIBRARY)

if(GMP_FOUND)
  if(NOT TARGET GMP::gmp)
    add_library(GMP::gmp UNKNOWN IMPORTED GLOBAL)
    set_target_properties(GMP::gmp PROPERTIES
      IMPORTED_LOCATION ${GMP_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${GMP_INCLUDE_DIR})
  endif()

  if(NOT TARGET GMP::gmpxx)
    add_library(GMP::gmpxx UNKNOWN IMPORTED GLOBAL)
    set_target_properties(GMP::gmpxx PROPERTIES
      IMPORTED_LOCATION ${GMPXX_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${GMPXX_INCLUDE_DIR}
      TARGET_LINK_LIBRARIES GMP::gmp)
  endif()
endif()
