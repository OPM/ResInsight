# - Find OPM corner-point grid library
#
# Defines the following variables:
#   opm-grid_INCLUDE_DIRS    Directory of header files
#   opm-grid_LIBRARIES       Directory of shared object files
#   opm-grid_DEFINITIONS     Defines that must be set to compile
#   opm-grid_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_GRID            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

include (opm-grid-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-grid"

  # dependencies
  "${opm-grid_DEPS}"
  
  # header to search for
  "dune/grid/CpGrid.hpp"

  # library to search for
  "opmgrid"

  # defines to be added to compilations
  "HAVE_OPM_GRID"

  # test program
"#include <dune/grid/CpGrid.hpp>
int main (void) {
  Dune::CpGrid g;
  return 0;
}
"
  # config variables
  "${opm-grid_CONFIG_VAR}"
  )

#debug_find_vars ("opm-grid")
