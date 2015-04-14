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

from ert.test import ExtendedTestCase
from ert.ecl import EclGrid,EclKW,EclTypeEnum,EclGrid,EclRegion
from ert.util import BoolVector


class DeprecationTest(ExtendedTestCase):

    def test_EclGrid_get_corner_xyz(self):
        grid = EclGrid.create_rectangular( (10,20,30) , (1,1,1) )
        with warnings.catch_warnings():
            grid.get_corner_xyz(0 , global_index = 10)
            
    def test_ecl_ecl_ecl(self):
        with warnings.catch_warnings():
            import ert.ecl.ecl as ecl

    # Added in 1.8.x development
    def test_EclKW_min_max(self):
        kw = EclKW.new("TEST", 3, EclTypeEnum.ECL_INT_TYPE)
        with warnings.catch_warnings():
            kw.min

        with warnings.catch_warnings():
            kw.max

        with warnings.catch_warnings():
            kw.min_max

    # Added in 1.8.x development
    def test_EclRegion_properties(self):
        grid = EclGrid.create_rectangular( (10,10,10) , (1,1,1))
        region = EclRegion( grid , False )

        with warnings.catch_warnings():
            region.active_size

        with warnings.catch_warnings():
            region.global_size

        with warnings.catch_warnings():
            region.global_list

        with warnings.catch_warnings():
            region.active_list



    # Deprecated method from 1.8.4
    def test_BoolVector_active_mask(self):
        with warnings.catch_warnings():
            active_vector = BoolVector.active_mask("1,1,1,1,1,1")