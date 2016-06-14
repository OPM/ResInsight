# - Find OPM automatic differentiation library
#
# Defines the following variables:
#   opm-simulators_INCLUDE_DIRS    Directory of header files
#   opm-simulators_LIBRARIES       Directory of shared object files
#   opm-simulators_DEFINITIONS     Defines that must be set to compile
#   opm-simulators_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_SIMULATORS            Binary value to use in config.h

# Copyright (C) 2012 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (opm-simulators-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-simulators"

  # dependencies
  "${opm-simulators_DEPS}"
  
  # header to search for
  "opm/autodiff/AutoDiff.hpp"

  # library to search for
  "opmsimulators"

  # defines to be added to compilations
  ""

  # test program
"#include <opm/autodiff/AutoDiff.hpp>
int main (void) {
  Opm::AutoDiff<double> x = Opm::AutoDiff<double>::constant(42.);
  (void) x;
  return 0;  
}
"
  # config variables
  "${opm-simulators_CONFIG_VAR}"
  )
include (UseDynamicBoost)
#debug_find_vars ("opm-simulators")
