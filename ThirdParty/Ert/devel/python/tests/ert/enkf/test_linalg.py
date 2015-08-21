#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_labscale.py' is part of ERT - Ensemble based Reservoir Tool.
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


from ert.enkf import ObsVector
from ert.util import Matrix
from ert.enkf.linalg import *


class LinalgTest(ExtendedTestCase):

    def test_num_PC(self):
        S = Matrix(3,3)
        s[0,0] = 1
        s[1,1] = 0.25
        s[2,2] = 0.125

        with self.assertRaises(ValueError):
            num_pc = numPC( S , -1 )


        with self.assertRaises(ValueError):
            num_pc = numPC( S , 1.5 )
    
        num_pc = numPC( S , 0.80 )
        
