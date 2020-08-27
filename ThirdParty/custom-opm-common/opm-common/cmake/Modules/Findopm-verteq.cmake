# - Find OPM vertical equilibrium library
#
# Defines the following variables:
#   opm-verteq_INCLUDE_DIRS    Directory of header files
#   opm-verteq_LIBRARIES       Directory of shared object files
#   opm-verteq_DEFINITIONS     Defines that must be set to compile
#   opm-verteq_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_VERTEQ            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (opm-verteq-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-verteq"

  # dependencies
  "${opm-verteq_DEPS}"

  # header to search for
  "opm/verteq/verteq.hpp"

  # library to search for
  "opmverteq"

  # defines to be added to compilations
  ""

  # test program
"#include <opm/verteq/verteq.hpp>
int main (void) {
  return 0;  
}
"
  # config variables
  "")
include (UseDynamicBoost)
#debug_find_vars ("opm-verteq")
