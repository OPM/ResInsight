#  Copyright (C) 2018  Statoil ASA, Norway.
#
#  The file 'test_grdecl.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ecl.eclfile import EclKW
from ecl.util.test import TestAreaContext
from tests import EclTest
import cwrap

class GRDECLTest(EclTest):

    def test_64bit_memory(self):
        with TestAreaContext("large_memory"):
            block_size = 10**6
            with open("test.grdecl","w") as f:
                f.write("COORD\n")
                for i in range(1000):
                    f.write("%d*0.15 \n" % block_size)
                f.write("/\n")

            with cwrap.open("test.grdecl") as f:
                kw = EclKW.read_grdecl(f,"COORD")


