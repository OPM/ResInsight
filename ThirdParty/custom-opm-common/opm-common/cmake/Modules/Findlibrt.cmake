# Module that makes the clock_gettime() function available
#
# Sets the following variables:
# HAVE_LIBRT
# LIBRT_LIBRARIES
#
# perform tests
include(CheckCSourceCompiles)

# first check if we need to add anything at all to be able to use
# clock_gettime()
CHECK_CXX_SOURCE_COMPILES("
#include <time.h>

int main()
{
    timespec time1;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
};
" HAVE_LIBRT)
cmake_pop_check_state()

if (HAVE_LIBRT)
  # if this worked, we're already happy
  set(LIBRT_LIBRARIES "")
else()
  # if not, let's try the same program with linking to librt (required
  # on some systems)
  cmake_push_check_state()
  list(APPEND CMAKE_REQUIRED_LIBRARIES "rt")
  CHECK_CXX_SOURCE_COMPILES("
#include <time.h>

int main()
{
    timespec time1;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
};
" HAVE_LIBRT2)
  cmake_pop_check_state()
  set(HAVE_LIBRT "${HAVE_LIBRT2}")
  if (HAVE_LIBRT)
    set(LIBRT_LIBRARIES "rt")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibRT
  DEFAULT_MSG
  HAVE_LIBRT
  )
