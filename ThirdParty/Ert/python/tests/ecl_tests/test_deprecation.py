#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_deprecation.py' is part of ERT - Ensemble based Reservoir Tool.
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
import warnings
import time
import datetime

from ecl.util.test import TestAreaContext
from ecl import EclDataType
from ecl.eclfile import EclFile, EclKW, FortIO, openFortIO
from ecl.grid import EclGrid, EclGrid, EclRegion, EclGridGenerator
from ecl.rft import EclRFT
from ecl.util.test.ecl_mock import createEclSum
from ecl.util.util import BoolVector
from tests import EclTest

# The class Deprecation_1_9_Test contains methods which will be marked
# as deprecated in the 1.9.x versions.

warnings.simplefilter("error" , DeprecationWarning)

class Deprecation_2_1_Test(EclTest):
    pass

class Deprecation_2_0_Test(EclTest):

    def test_EclFile_name_property(self):
        with TestAreaContext("name") as t:
            kw = EclKW("TEST", 3, EclDataType.ECL_INT)
            with openFortIO("TEST" , mode = FortIO.WRITE_MODE) as f:
                kw.fwrite( f )

            t.sync()
            f = EclFile( "TEST" )

class Deprecation_1_9_Test(EclTest):

    def test_EclRegion_properties(self):
        grid = EclGridGenerator.createRectangular( (10,10,10) , (1,1,1))
        region = EclRegion( grid , False )
