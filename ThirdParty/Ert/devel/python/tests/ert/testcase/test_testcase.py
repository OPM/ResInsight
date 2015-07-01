#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'test_testcase.py' is part of ERT - Ensemble based Reservoir Tool.
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

import functools
import math
from ert.test import ExtendedTestCase

class TestTestCase(ExtendedTestCase):
    
    def test_not_raises(self):
        call_sin = functools.partial( math.sin , 0.5*math.pi )
        self.assertNotRaises( call_sin )
        
