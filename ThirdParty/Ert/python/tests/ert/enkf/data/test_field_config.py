#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_field_config.py' is part of ERT - Ensemble based Reservoir Tool.
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

import os

from ert.enkf import FieldConfig
from ert.enkf import ActiveList
from ert.test import ExtendedTestCase
from ert.ecl  import EclGrid

class FieldConfigTest(ExtendedTestCase):

    def test_create(self):
        grid = EclGrid.createRectangular( (10,10,5) , (1,1,1) )
        field_config = FieldConfig("SWAT" , grid)
        
    
    
