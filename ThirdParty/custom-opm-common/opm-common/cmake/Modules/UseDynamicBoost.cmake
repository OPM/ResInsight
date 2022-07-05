# When using Boost >= 1.70 and e.g. CMake 3.18 we need to make sure that
# subsequent searches are using config mode too. Otherwise the library
# list will be completely messed up. We use a set Boost_Dir to detect that
# previous searches were done using config mode.
if(Boost_DIR)
  set(_Boost_CONFIG_MODE CONFIG)
endif()
find_package (Boost 1.44.0 COMPONENTS unit_test_framework QUIET ${_Boost_CONFIG_MODE})

if (Boost_UNIT_TEST_FRAMEWORK_FOUND)
  # setup to do a test compile
  include (CMakePushCheckState)
  cmake_push_check_state ()
  include (CheckCXXSourceCompiles)
  list (APPEND CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIRS})
  list (APPEND CMAKE_REQUIRED_LIBRARIES ${Boost_LIBRARIES})

  unset(HAVE_DYNAMIC_BOOST_TEST CACHE)
  check_cxx_source_compiles("
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE DYNLINK_TEST
#include <boost/test/unit_test.hpp>

int f(int x) { return 2 * x; }

BOOST_AUTO_TEST_CASE(DynlinkConfigureTest) {
  BOOST_CHECK_MESSAGE(f(2) == 4,
                      \"Apparently, multiplication doesn't \"
                      \"work: f(2) = \" << f(2));
}" HAVE_DYNAMIC_BOOST_TEST)
  cmake_pop_check_state ()
else (Boost_UNIT_TEST_FRAMEWORK_FOUND)
  # no Boost, no compile
  set (HAVE_DYNAMIC_BOOST_TEST 0)
endif (Boost_UNIT_TEST_FRAMEWORK_FOUND)

# save this for later
set (HAVE_DYNAMIC_BOOST_TEST "${HAVE_DYNAMIC_BOOST_TEST}"
  CACHE BOOL "Whether Boost::Test is dynamically linked or not"
  )

# include in config.h
list (APPEND TESTING_CONFIG_VARS "HAVE_DYNAMIC_BOOST_TEST")
