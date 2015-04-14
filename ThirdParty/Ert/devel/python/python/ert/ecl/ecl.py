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
# """
# import libecl
# from   ecl_kw                import EclKW
# from   ecl_case              import EclCase
# from   ecl_file              import EclFile
# from   ecl_sum               import EclSum
# from   ecl_rft               import EclRFTFile , EclRFT
# from   ecl_rft_cell          import EclRFTCell, EclPLTCell
# from   ecl_grid              import EclGrid
# from   ecl_grav              import EclGrav
# from   ecl_subsidence        import EclSubsidence
# from   ecl_region            import EclRegion
# from   fortio                import FortIO
# from   ecl_queue             import EclQueue
# import ecl_util
# from   ecl_util              import *
#
# import ecl_default
import os

warning_message = """

The ecl namespace is deprecated and will go away.  You are advised to
use one of these alternatives when importing the ert.ecl modules and
classes:

---------------------------------------------------------------------
Common prefix:
import ert.ecl as ecl  

Direct import of classes:
from ert.ecl import EclGrid,EclSum
---------------------------------------------------------------------

The environment variable ERT_SILENCE_WARNINGS is a colon separated
list which can be used to silence warnings. To silence this particular
warning set:

ERT_SILENCE_WARNINGS = \"eclimport\"

"""


from warnings import warn

ert_silence_warnings = os.environ.get("ERT_SILENCE_WARNINGS")
if ert_silence_warnings:
    silent_warnings = set( ert_silence_warnings.split(":"))
else:
    silent_warnings = set()

if not "eclimport" in silent_warnings:
    warn(warning_message)


from .ecl_sum import EclSum #, EclSumVector, EclSumNode, EclSMSPECNode
from .ecl_rft_cell import EclPLTCell, EclRFTCell
from .ecl_rft import EclRFT, EclRFTFile
from .ecl_kw import EclKW
from .ecl_file import EclFile
from .fortio import FortIO
from .ecl_grid import EclGrid
from .ecl_region import EclRegion
from .ecl_case import EclCase
from .ecl_subsidence import EclSubsidence
from .ecl_grav_calc import deltag, phase_deltag
from .ecl_grav import EclGrav

# The EclQueue class uses the libjob_queue library which is only built
# when the full ert distribution is built. If BUILD_ERT == False the
# ecl_queue module is excluded from the build process.
try:
    from .ecl_queue import EclQueue
except ImportError:
    pass

from .ecl_default import default


#Legacy import support
class default_wrapper(object):
    default = default

ecl_default = default_wrapper()


from .ecl_util import EclFileEnum, EclFileFlagEnum, EclPhaseEnum, EclTypeEnum, EclUtil

#make enum values globally available in ert.ecl.ecl
for enum in EclFileEnum.enum_names:
    globals()[enum] = getattr(EclFileEnum, enum)

for enum in EclFileFlagEnum.enum_names:
    globals()[enum] = getattr(EclFileFlagEnum, enum)

for enum in EclPhaseEnum.enum_names:
    globals()[enum] = getattr(EclPhaseEnum, enum)

for enum in EclTypeEnum.enum_names:
    globals()[enum] = getattr(EclTypeEnum, enum)


        



        
    
