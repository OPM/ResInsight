include (OpmPackage)

find_opm_package (
  # module name
  "dune-uggrid"

  # dependencies
  # TODO: we should probe for all the HAVE_* values listed below;
  # however, we don't actually use them in our implementation, so
  "dune-common REQUIRED
  "
  # header to search for
  ""

  # library to search for
  "duneuggrid"

  # defines to be added to compilations
  ""

  # test program
  ""
  # config variable
  "")

#debug_find_vars ("dune-uggrid")

# make version number available in config.h
include (UseDuneVer)
find_dune_version ("dune" "uggrid")

# deactivate search for UG
set(UG_FOUND ${dune-uggrid_FOUND})
