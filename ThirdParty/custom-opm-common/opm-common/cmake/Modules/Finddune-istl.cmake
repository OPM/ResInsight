# - Find DUNE ISTL library
#
# Defines the following variables:
#   dune-istl_INCLUDE_DIRS      Directory of header files
#   dune-istl_LIBRARIES         Directory of shared object files
#   dune-istl_DEFINITIONS       Defines that must be set to compile
#   dune-istl_CONFIG_VARS       List of defines that should be in config.h
#   HAVE_DUNE_ISTL              Binary value to use in config.h

# Copyright (C) 2012 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

include (OpmPackage)
find_opm_package (
  # module name
  "dune-istl"

  # required dependencies
  "dune-common REQUIRED;
  ParMETIS;
  SuperLU;
  SuiteSparse COMPONENTS umfpack REQUIRED
  "
  # header to search for
  "dune/istl/bcrsmatrix.hh"

  # library to search for
  ""

  # defines to be added to compilations
  ""

  # test program
"#include <dune/common/deprecated.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>

int main (void) {
  typedef Dune::BCRSMatrix<Dune::FieldMatrix<double,1,1> > Matrix;
  Matrix matrix( 3, 3, Matrix::random );
  for (int i = 0; i < 3; ++i) matrix.setrowsize(i, 2);
  matrix.endrowsizes();
  return 0;
}
"
  # config variables
  "HAVE_BOOST_FUSION;
  HAVE_MEM_USAGE_T_EXPANSIONS;
  HAVE_PARDISO;
  HAVE_BOOST;
  HAVE_MPI;
  HAVE_PARMETIS;
  HAVE_SUPERLU;
  HAVE_UMFPACK;
  HAVE_SUITESPARSE_UMFPACK;
  SUPERLU_INT_TYPE;
  HAVE_SLU_CDEFS_H;
  HAVE_SLU_DDEFS_H;
  HAVE_SLU_SDEFS_H;
  HAVE_SLU_ZDEFS_H;
  SUPERLU_MIN_VERSION_4_3;
  SUPERLU_MIN_VERSION_5;
  SUPERLU_POST_2005_VERSION
  ")
#debug_find_vars ("dune-istl")

# make version number available in config.h
include (UseDuneVer)
find_dune_version ("dune" "istl")
