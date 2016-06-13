# - Find DUNE grid library
#
# Defines the following variables:
#   dune-grid_INCLUDE_DIRS    Directory of header files
#   dune-grid_LIBRARIES       Directory of shared object files
#   dune-grid_DEFINITIONS     Defines that must be set to compile
#   dune-grid_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_DUNE_GRID            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

include (OpmPackage)

set(DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS 1)

find_opm_package (
  # module name
  "dune-grid"

  # dependencies
  # TODO: we should probe for all the HAVE_* values listed below;
  # however, we don't actually use them in our implementation, so
  # we just include them to forward here in case anyone else does
  "CXX11Features REQUIRED;
  dune-common REQUIRED;
  dune-geometry REQUIRED;
  MPI;
  ALUGrid;
  UG
  "
  # header to search for
  "dune/grid/onedgrid.hh"

  # library to search for
  "dunegrid"

  # defines to be added to compilations
  ""

  # test program
"#include <dune/grid/onedgrid.hh>
int main (void) {
  Dune::OneDGrid grid(1, 0., 1.);
  return grid.lbegin<0>(0) == grid.lend<0>(0);
}
"
  # config variables
  "HAVE_MPI;
   HAVE_UG;
   HAVE_DUNE_FEM;
   HAVE_ALUGRID;
   HAVE_GRIDTYPE;
   HAVE_GRAPE;
   HAVE_PSURFACE;
   HAVE_AMIRAMESH;
   HAVE_ALBERTA;
   HAVE_STDINT_H;
   DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS
  ")

#debug_find_vars ("dune-grid")

# make version number available in config.h
include (UseDuneVer)
find_dune_version ("dune" "grid")
