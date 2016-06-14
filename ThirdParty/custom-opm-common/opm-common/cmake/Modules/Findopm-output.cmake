# - Find OPM output library
#
# Defines the following variables:
#   opm-output_INCLUDE_DIRS    Directory of header files
#   opm-output_LIBRARIES       Directory of shared object files
#   opm-output_DEFINITIONS     Defines that must be set to compile
#   opm-output_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_OUTPUT            Binary value to use in config.h

# Copyright (C) 2012 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (opm-output-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-output"

  # dependencies
  "${opm-output_DEPS}"
  
  # header to search for
  "opm/output/OutputWriter.hpp"

  # library to search for
  "opmoutput"

  # defines to be added to compilations
  ""

  # test program
"#include <opm/output/OutputWriter.hpp>
int main (void) {
  return 0;  
}
"
  # config variables
  "${opm-output_CONFIG_VAR}"
  )
include (UseDynamicBoost)
#debug_find_vars ("opm-output")
