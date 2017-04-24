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

  ecl_type/EclDataType: This class is used to represent the data type
    of the elements in EclKW.

  ecl_file/EclFile: This class is used to load an ECLIPSE file in
     restart format, alternatively only parts of the file can be
     loaded. Internally it consists of a collection of EclKW
     instances.

  ecl_grid/EclGrid: This will load an ECLIPSE GRID or EGRID file, and
     can then subsequently be used for queries about the grid.

  ecl_grid_generator/EclGridGenerator: This can be used to generate various
    grids.

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

In addition there are some modules which do not follow the one class
per module organization:

  ecl_util: This is mainly a collection of constants, and a few
     stateless functions.

  ecl: This module is purely for convenience, all the symbols in the
     package are explicitly imported into this module, ensuring that
     all symbols in the package are available under the common
     namespace 'ecl'.

"""
import ert.util
import ert.geo

from cwrap import Prototype


class EclPrototype(Prototype):
    lib = ert.load("libecl")

    def __init__(self, prototype, bind=True):
        super(EclPrototype, self).__init__(EclPrototype.lib, prototype, bind=bind)

ECL_LIB = ert.load("libecl")

from .ecl_util import EclFileEnum, EclFileFlagEnum, EclPhaseEnum, EclUnitTypeEnum , EclUtil
from .ecl_type import EclTypeEnum, EclDataType
from .ecl_sum_var_type import EclSumVarType
from .ecl_sum_tstep import EclSumTStep
from .ecl_sum import EclSum #, EclSumVector, EclSumNode, EclSMSPECNode
from .ecl_sum_keyword_vector import EclSumKeyWordVector
from .ecl_rft_cell import EclPLTCell, EclRFTCell
from .ecl_rft import EclRFT, EclRFTFile
from .fortio import FortIO, openFortIO
from .ecl_kw import EclKW
from .ecl_3dkw import Ecl3DKW
from .ecl_file_view import EclFileView
from .ecl_file import EclFile , openEclFile
from .ecl_3d_file import Ecl3DFile
from .ecl_init_file import EclInitFile
from .ecl_restart_file import EclRestartFile
from .ecl_grid import EclGrid
from .ecl_region import EclRegion
from .ecl_subsidence import EclSubsidence
from .ecl_grav_calc import phase_deltag, deltag
from .ecl_grav import EclGrav
from .ecl_sum_node import EclSumNode
from .ecl_sum_vector import EclSumVector
from .ecl_npv import EclNPV , NPVPriceVector
from .ecl_cmp import EclCmp
from .ecl_grid_generator import EclGridGenerator
