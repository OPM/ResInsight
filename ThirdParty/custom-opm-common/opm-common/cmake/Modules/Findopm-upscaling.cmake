# - Find OPM upscaling grid library
#
# Defines the following variables:
#   opm-upscaling_INCLUDE_DIRS    Directory of header files
#   opm-upscaling_LIBRARIES       Directory of shared object files
#   opm-upscaling_DEFINITIONS     Defines that must be set to compile
#   opm-upscaling_CONFIG_VARS     List of defines that should be in config.h
#   HAVE_OPM_UPSCALING            Binary value to use in config.h

# Copyright (C) 2013 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

include (opm-upscaling-prereqs)
include (OpmPackage)
find_opm_package (
  # module name
  "opm-upscaling"

  # dependencies
  "${opm-upscaling_DEPS}"

  # header to search for
  "opm/upscaling/SinglePhaseUpscaler.hpp"

  # library to search for
  "opmupscaling"

  # defines to be added to compilations
  ""

  # test program
"#include <opm/upscaling/SinglePhaseUpscaler.hpp>
int main (void) {
  return 0;
}
"
  # config variables
  "${opm-upscaling_CONFIG_VAR}"
  )

#debug_find_vars ("opm-upscaling")
