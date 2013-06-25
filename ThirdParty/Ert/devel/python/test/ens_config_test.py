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
from   test_util import approx_equal, approx_equalv

case = "../../../libenkf/src/Gurbat/enkf.ext"

class EnsConfigTest( unittest.TestCase ):
    def setUp(self):
        pass

    def test_key(self):
        main = enkf.EnKFMain.bootstrap( case )
        conf = main.config
        self.assertTrue( conf.has_key("WWCT:OP_1" ))
        self.assertFalse( conf.has_key("WWCT:OP_1X" ))

    

unittest.main()
