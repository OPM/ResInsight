#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_layer.py' is part of ERT - Ensemble based Reservoir Tool.
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
try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

import time
from ert.ecl.faults import Layer
from ert.test import ExtendedTestCase

class LayerTest(ExtendedTestCase):
    def setUp(self):
        pass


    def test_create_layer(self):
        layer = Layer(10,10)
        self.assertTrue( isinstance( layer , Layer ))


    def test_add_cell(self):
        layer = Layer(10,10)
        with self.assertRaises(ValueError):
            layer[100,100] = 199

        with self.assertRaises(ValueError):
            layer[100,"X"] = 199

        with self.assertRaises(ValueError):
            layer[100] = 199

        layer[5,5] = 88
        self.assertEqual(layer[5,5] , 88)
        
