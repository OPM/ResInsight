# - Find OPM materials library
#
# Defines the following variables:
#   opm-material_INCLUDE_DIRS    Directory of header files
#   opm-material_LIBRARIES       Directory of shared object files
#   opm-material_DEFINITIONS     Defines that must be set to compile
#   opm-material_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_MATERIAL            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (opm-material-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-material"

  # dependencies
  "${opm-material_DEPS}"

  # header to search for
  "opm/material/Constants.hpp"

  # library to search for
  ""

  # defines to be added to compilations
  ""

  # test program
"#include <opm/material/Constants.hpp>
int main (void) {
  double c = Opm::Constants<double>::c;
  return 0;  
}
"
  # config variables
  "${opm-material_CONFIG_VAR}"
  )
include (UseDynamicBoost)
#debug_find_vars ("opm-material")
