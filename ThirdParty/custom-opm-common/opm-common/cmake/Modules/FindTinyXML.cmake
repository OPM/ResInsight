# - Find TinyXML library
#
# Defines the following variables:
#   TinyXML_INCLUDE_DIRS    Directory of header files
#   TinyXML_LIBRARIES       Directory of shared object files
#   TinyXML_DEFINITIONS     Defines that must be set to compile

# Copyright (C) 2012 Uni Research AS
# This code is licensed under The GNU General Public License v3.0

# use the generic find routine
include (OpmPackage)
find_opm_package (
  # module name
  "TinyXML"

  # dependencies
  ""
  
  # header to search for
  "tinyxml.h"

  # library to search for
  "tinyxml"

  # defines to be added to compilations
  ""

  # test program
"#include <tinyxml.h>
int main (void) {
  TiXmlDocument doc;
  return 0;  
}
"
  # config variables
  "")
