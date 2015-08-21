#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_block_obs.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.enkf import BlockObservation
from ert.enkf import ActiveList, FieldConfig 
from ert.test import ExtendedTestCase
from ert.ecl import EclGrid


class BlockObsTest(ExtendedTestCase):

    def test_create(self):
        grid = EclGrid.createRectangular( (10,20,5) , (1,1,1) )
        field_config = FieldConfig("PRESSURE" , grid)    
        block_obs = BlockObservation("P-CONFIG" , field_config , grid)

        self.assertEqual(  len(block_obs) , 0 )

        block_obs.addPoint(1,2,3,100,25)
        self.assertEqual(  len(block_obs) , 1 )
        self.assertEqual( block_obs.getValue(0) , 100 )
        self.assertEqual( block_obs.getStd(0) , 25 )
        self.assertEqual( block_obs.getStdScaling(0) , 1 )

        block_obs.addPoint(1,2,4,200,50)
        self.assertEqual(  len(block_obs) , 2 )
        self.assertEqual( block_obs.getValue(1) , 200 )
        self.assertEqual( block_obs.getStd(1) , 50 )
        self.assertEqual( block_obs.getStdScaling(1) , 1 )

        active_list = ActiveList( )
        block_obs.updateStdScaling( 0.50 , active_list )
        self.assertEqual( block_obs.getStdScaling(0) , 0.50 )
        self.assertEqual( block_obs.getStdScaling(1) , 0.50 )

        active_list.addActiveIndex( 1 )
        block_obs.updateStdScaling( 2.00 , active_list )
        self.assertEqual( block_obs.getStdScaling(0) , 0.50 )
        self.assertEqual( block_obs.getStdScaling(1) , 2.00 )
        
