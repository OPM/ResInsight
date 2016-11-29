# - Find the opm-common module
#
# Defines the following variables:
#   opm-common_INCLUDE_DIRS    Directory of header files
#   opm-common_LIBRARIES       Directory of shared object files
#   opm-common_DEFINITIONS     Defines that must be set to compile
#   opm-common_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_COMMON            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (opm-common-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-common"

  # dependencies
  "${opm-common_DEPS}"

  # header to search for
  "opm/common/utility/platform_dependent/disable_warnings.h"

  # library to search for
  "opmcommon"

  # defines to be added to compilations
  ""

  # test program
  "#include <opm/common/utility/platform_dependent/disable_warnings.h>
int main (void) {
  return 0;  
}
"

  # config variables
  "${opm-common_CONFIG_VAR}"
  )
include (UseDynamicBoost)
#debug_find_vars ("opm-common")
