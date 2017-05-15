#!/usr/bin/env python
#  Copyright (C) 2017  Statoil ASA, Norway. 
#   
#  The file 'test_region.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ecl.ecl import EclGrid, EclKW, EclRegion, EclDataType
from ecl.ecl.faults import Layer
from ecl.test import ExtendedTestCase


class RegionTest(ExtendedTestCase):

    def test_equal(self):
        grid = EclGrid.createRectangular( (10,10,1) , (1,1,1))
        kw_int = EclKW( "INT" , grid.getGlobalSize( ) , EclDataType.ECL_INT )
        kw_float = EclKW( "FLOAT" , grid.getGlobalSize( ) , EclDataType.ECL_FLOAT )

        kw_int[0:49] = 1
        region = EclRegion(grid, False)
        region.select_equal( kw_int , 1 )
        glist = region.getGlobalList()
        for g in glist:
            self.assertEqual( kw_int[g] , 1 )

        with self.assertRaises(ValueError):
            region.select_equal( kw_float , 1 )
