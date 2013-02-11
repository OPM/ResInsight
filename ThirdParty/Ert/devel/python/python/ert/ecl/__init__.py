#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file '__init__.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 
"""
Package for working with ECLIPSE files.

The ecl package contains several classes for working with ECLIPSE
files. The ecl package is a wrapper around the libecl library from the
ERT distribution. Mainly the package is organized with modules
ecl_xxx.py with a class EclXXX. The module ecl_xxx.py will generaly
wrap the content of the c-file ecl_xxx.c The main content is:

  fortio/FortIO: This is functionality to read and write binary
     fortran files.

  ecl_kw/EclKW: This class holds one ECLIPSE keyword, like SWAT, in
     restart format.

  ecl_file/EclFile: This class is used to load an ECLIPSE file in
     restart format, alternatively only parts of the file can be
     loaded. Internally it consists of a collection of EclKW
     instances.

  ecl_grid/EclGrid: This will load an ECLIPSE GRID or EGRID file, and
     can then subsequently be used for queries about the grid.

  ecl_sum/EclSum: This will load summary results from an ECLIPSE run;
     both data file(s) and the SMSPEC file. The EclSum object can be
     used as basis for queries on summary vectors.

  ecl_rft/[EclRFTFile , EclRFT , EclRFTCell]: Loads an ECLIPSE RFT/PLT
     file, and can afterwords be used to support various queries.

  ecl_region/EclRegion: Convenience class to support selecting cells
     in a grid based on a wide range of criteria. Can be used as a
     mask in operations on EclKW instances.

  ecl_grav/EclGrav: Class used to simplify evaluation of ECLIPSE
     modelling time-lapse gravitational surveys.

  ecl_subsidence/EclSubsidence: Small class used to evaluate simulated
     subsidence from ECLIPSE simulations; analogous to the EcLGrav
     functionality.

  ecl_queue/EclQueue: Class implementing a queue to run ECLIPSE
     simulations.

In addition there are some modules which do not follow the one class
per module organization:

  ecl_util: This is mainly a collection of constants, and a few
     stateless functions.

  ecl: This module is purely for convenience, all the symbols in the
     package are explicitly imported into this module, ensuring that
     all symbols in the package are available under the common
     namespace 'ecl'.

  libecl: This module contains some low-level ctypes trickery to
     actually load the shared library libecl.so.
"""

