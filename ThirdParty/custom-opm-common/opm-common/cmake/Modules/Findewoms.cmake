# - Find OPM eWoms module
#
# Defines the following variables:
#   ewoms_INCLUDE_DIRS    Directory of header files
#   ewoms_LIBRARIES       Directory of shared object files
#   ewoms_DEFINITIONS     Defines that must be set to compile
#   ewoms_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_EWOMS            Binary value to use in config.h

# Copyright (C) 2012 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (ewoms-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "ewoms"

  # dependencies
  "${ewoms_DEPS}"
  
  # header to search for
  "ewoms/common/start.hh"

  # library to search for
  ""

  # defines to be added to compilations
  ""

  # test program
"#include <ewoms/common/start.hh>
int main (void) {
return 0;
}
"
  # config variables
  "${ewoms_CONFIG_VAR}"
  )
#include (UseDynamicBoost)
#debug_find_vars ("ewoms")
