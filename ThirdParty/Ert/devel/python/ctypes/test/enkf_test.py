#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'sum_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import datetime
import unittest
import ert
import ert.enkf.enkf as enkf
from   ert.util.tvector import * 
from   test_util import approx_equal, approx_equalv

case = "../../../libenkf/src/Gurbat/enkf.ext"

class EnKFtest( unittest.TestCase ):
    def setUp(self):
        pass


    def test_boot( self ):
        self.main = enkf.EnKFMain.bootstrap( case )
        self.assertTrue( self.main , "Load failed" )
        del self


    def test_enum(self):
        self.assertEqual( enkf.enkf_state_enum.FORECAST , 2 )
        self.assertEqual( enkf.enkf_state_enum.ANALYZED , 4 )

        
    def test_config( self ):
        self.main = enkf.EnKFMain.bootstrap( case )
        config = self.main.config
        self.assertTrue( isinstance( config , ert.enkf.ens_config.EnsConfig))

    def test_update(self):
        step_list = IntVector(0)
        step_list.append(30)
        self.main = enkf.EnKFMain.bootstrap( case )
        self.main.update(step_list)

        
    #def test_sim(self):
    #    self.main = enkf.EnKFMain.bootstrap( case )
    #    self.main.sim()

unittest.main()
