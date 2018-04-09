#!/usr/bin/env python
#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_kw_function.py' is part of ERT - Ensemble based Reservoir Tool.
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
import random
from ecl import EclDataType
from ecl.eclfile import EclKW, Ecl3DKW
from ecl.grid import EclGrid
from ecl.util.util import IntVector
from tests import EclTest

class KWFunctionTest(EclTest):

    def test_region_filter(self):
        nx = 10
        ny = 10
        nz = 1
        actnum = IntVector( initial_size = nx*ny*nz , default_value = 1 )
        actnum[nx*ny - 1] = 0

        grid = EclGrid.createRectangular( (nx,ny,nz) , (1,1,1) , actnum = actnum)
        self.assertEqual( grid.getNumActive() , nx*ny*nz - 1 )

        kw = Ecl3DKW.create( "REGIONS" , grid , EclDataType.ECL_INT , global_active = True )
        kw.assign( 0 )
        kw[0:int(nx*ny/2)] = 1
        kw[5,2,0] = 0
        kw[0,9,0] = 2

        kw.fixUninitialized( grid )

        # Not assigned because they are in contact with a '2'; these
        # two are problem cells.
        self.assertEqual( kw[0,ny - 2,0] , 0)
        self.assertEqual( kw[1,ny - 1,0] , 0)

        # Not assigned because it is inactive
        self.assertEqual( kw[nx - 1,ny - 1,0] , 0)

        self.assertEqual( kw[5,2,0] , 1 )
        for j in range(5,10):
            self.assertEqual( kw[5,j,0] , 1 )

        for i in range(10):
            self.assertEqual( kw[i,7,0] , 1 )



    def test_porv_kw(self):
        porv_int = EclKW( "PORV", 100, EclDataType.ECL_INT)
        with self.assertRaises(TypeError):
            actnum = porv_int.create_actnum()


        prv = EclKW("PRV", 100, EclDataType.ECL_FLOAT)
        with self.assertRaises(ValueError):
            actnum = prv.create_actnum()


        porv = EclKW("PORV", 4, EclDataType.ECL_FLOAT)
        porv[0] = 0
        porv[1] = 0.50
        porv[2] = 0.50
        porv[3] = 0
        actnum = porv.create_actnum()
        self.assertEqual(tuple(actnum), (0,1,1,0))
