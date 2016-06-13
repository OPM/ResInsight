# - Find OPM core library
#
# Defines the following variables:
#   opm-core_INCLUDE_DIRS    Directory of header files
#   opm-core_LIBRARIES       Directory of shared object files
#   opm-core_DEFINITIONS     Defines that must be set to compile
#   opm-core_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_CORE            Binary value to use in config.h

# Copyright (C) 2012 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (opm-core-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-core"

  # dependencies
  "${opm-core_DEPS}"
  
  # header to search for
  "opm/core/grid.h"

  # library to search for
  "opmcore"

  # defines to be added to compilations
  ""

  # test program
"#include <opm/core/grid.h>
int main (void) {
  struct UnstructuredGrid *g;
  g = create_grid_empty ();
  destroy_grid (g);
  return 0;  
}
"
  # config variables
  "${opm-core_CONFIG_VAR}"
  )
include (UseDynamicBoost)
#debug_find_vars ("opm-core")
