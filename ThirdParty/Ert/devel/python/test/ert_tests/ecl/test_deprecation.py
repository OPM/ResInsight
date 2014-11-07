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

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

import time
from ert.ecl import EclGrid
from ert.test import ExtendedTestCase


class DeprecationTest(ExtendedTestCase):

    def test_EclGrid_get_corner_xyz(self):
        grid = EclGrid.create_rectangular( (10,20,30) , (1,1,1) )
        with warnings.catch_warnings():
            grid.get_corner_xyz(0 , global_index = 10)
            
            
