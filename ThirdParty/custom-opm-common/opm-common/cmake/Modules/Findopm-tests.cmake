# This module searches for the opm-tests repository. Since opm-tests
# has no libraries or header files the find implementation is quite
# naive.
#
# If the opm-tests repository is found, the following variables are set:
#
#   HAVE_OPM_TESTS
#   OPM_TESTS_ROOT

if (OPM_TESTS_ROOT)
   set( _opm_tests_root ${OPM_TESTS_ROOT})
else()
   set( _opm_tests_root "${PROJECT_SOURCE_DIR}/../opm-tests")
endif()


if (EXISTS "${_opm_tests_root}/norne/NORNE_ATW2013.DATA")
   set( HAVE_OPM_DATA True )
   set( HAVE_OPM_TESTS True )
   set( OPM_TESTS_ROOT ${_opm_tests_root} )
   set( OPM_DATA_ROOT ${_opm_tests_root} )
   message(STATUS "Setting OPM_TESTS_ROOT: ${OPM_TESTS_ROOT}")
else()
   set( HAVE_OPM_TESTS False )
   set( HAVE_OPM_DATA False )
   message(WARNING "opm-tests not found - integration tests using opm-tests will be skipped.")
endif()
