#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Convenience module importing all symbols in the ecl package.

This module is purely for convenience; it explicitly imports all the
symbols from all the other modules into the common 'ecl'
namespace. The whole point of this excercise is to facilitate the
following construction:

   import ert.ecl.ecl as ecl            <-- Import this module; and create the
                                            namespace 'ecl'
   ...
   ...
   sum  = ecl.EclSum( "ECLIPSE.DATA")   <-- Classes EclSum and EclGrid are now
   grid = ecl.EclGrid("ECLIPSE.EGRID")      accessible under the ecl namespace.
   
It is not necessary to use the this module. See the doc/import.txt
document in the ert-python source distribution for more details of
module import and namespace resolution.
"""
import libecl
from   ecl_kw                import EclKW
from   ecl_case              import EclCase
from   ecl_file              import EclFile
from   ecl_sum               import EclSum
from   ecl_rft               import EclRFTFile , EclRFT , EclRFTCell
from   ecl_grid              import EclGrid
from   ecl_grav              import EclGrav
from   ecl_subsidence        import EclSubsidence
from   ecl_region            import EclRegion
from   fortio                import FortIO
from   ecl_queue             import EclQueue
import ecl_util
from   ecl_util              import *  

import ecl_default




        



        
    
