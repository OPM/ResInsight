# This module searches for the opm-data repository. Since the opm-data
# has no libraries or header files the find implementation is quite
# naive.
#
# If the opm-data repository is found, the following variables are set:
#
#   HAVE_OPM_DATA
#   OPM_DATA_ROOT

if (OPM_DATA_ROOT)
   set( _opm_data_root ${OPM_DATA_ROOT})
else()
   set( _opm_data_root "${PROJECT_SOURCE_DIR}/../opm-data")
endif()


if (EXISTS "${_opm_data_root}/norne/NORNE_ATW2013.DATA")
   set( HAVE_OPM_DATA True )
   set( OPM_DATA_ROOT ${_opm_data_root} )
   message( "-- Setting OPM_DATA_ROOT: ${OPM_DATA_ROOT}")
else()
   set( HAVE_OPM_DATA False )
   message( "opm-data not found - integration tests using opm-data will be skipped.")
endif()