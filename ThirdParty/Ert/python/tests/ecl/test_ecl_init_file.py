#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_ecl_init_file.py' is part of ERT - Ensemble based Reservoir Tool.
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


from ecl.test import ExtendedTestCase
from ecl.ecl import (Ecl3DKW, EclKW, EclInitFile, EclFile, FortIO,
                     EclFileFlagEnum, EclGrid)

class InitFileTest(ExtendedTestCase):


    def setUp(self):
        self.grid_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")
        self.init_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.INIT")


    def test_wrong_type(self):
        g = EclGrid(self.grid_file)
        with self.assertRaises(ValueError):
            f = EclInitFile(g, self.grid_file)


    def test_load(self):
        g = EclGrid(self.grid_file)
        f = EclInitFile(g, self.init_file)

        head = f["INTEHEAD"][0]
        self.assertTrue(isinstance(head, EclKW))

        porv = f["PORV"][0]
        self.assertTrue(isinstance(porv, Ecl3DKW))

        poro = f["PORO"][0]
        self.assertTrue(isinstance(poro, Ecl3DKW))
