# - Find DUNE geometry library
#
# Defines the following variables:
#   dune-geometry_INCLUDE_DIRS    Directory of header files
#   dune-geometry_LIBRARIES       Directory of shared object files
#   dune-geometry_DEFINITIONS     Defines that must be set to compile
#   dune-geometry_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_DUNE_GEOMETRY            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

include (OpmPackage)
find_opm_package (
  # module name
  "dune-geometry"

  # dependencies
  # TODO: we should probe for all the HAVE_* values listed below;
  # however, we don't actually use them in our implementation, so
  # we just include them to forward here in case anyone else does
  "CXX11Features REQUIRED;
  dune-common REQUIRED
  "
  # header to search for
  "dune/geometry/quadraturerules.hh"

  # library to search for
  "dunegeometry"

  # defines to be added to compilations
  ""

  # test program
"#include <dune/geometry/quadraturerules.hh>
int main (void) {
  Dune::GeometryType gt;
  gt.makeQuadrilateral();
  Dune::QuadratureRules<double, 2>::rule(gt, 2).size();
  return 0;
}
"
  # config variables
  "HAVE_ALGLIB
  ")

#debug_find_vars ("dune-geometry")

# make version number available in config.h
include (UseDuneVer)
find_dune_version ("dune" "geometry")
