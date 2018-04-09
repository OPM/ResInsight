#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway.
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

import os
from ecl.eclfile import EclKW, Ecl3DKW
from ecl.grid import EclGrid
from tests import EclTest, statoil_test

from cwrap import open as copen


@statoil_test()
class GRDECLTest(EclTest):
    def setUp(self):
        self.src_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/include/example_permx.GRDECL")
        self.file_list = []

    def addFile(self, filename):
        self.file_list.append(filename)

    def tearDown(self):
        for f in self.file_list:
            if os.path.exists(f):
                os.unlink(f)


    def test_Load( self ):
        kw = EclKW.read_grdecl(copen(self.src_file, "r"), "PERMX")
        self.assertTrue(kw)

        grid = EclGrid( self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE" ))
        kw = Ecl3DKW.read_grdecl(grid , copen(self.src_file, "r"), "PERMX")
        self.assertTrue( isinstance( kw , Ecl3DKW ))



    def test_reload( self ):
        kw = EclKW.read_grdecl(copen(self.src_file, "r"), "PERMX")
        tmp_file1 = "/tmp/permx1.grdecl"
        tmp_file2 = "/tmp/permx2.grdecl"
        self.addFile(tmp_file1)
        self.addFile(tmp_file2)

        fileH = copen(tmp_file1, "w")
        kw.write_grdecl(fileH)
        fileH.close()

        kw1 = EclKW.read_grdecl(copen(tmp_file1, "r"), "PERMX")

        fileH = copen(tmp_file2, "w")
        kw1.write_grdecl(fileH)
        fileH.close()

        self.assertFilesAreEqual(tmp_file1, tmp_file2)


    def test_fseek( self ):
        file = copen(self.src_file, "r")
        self.assertTrue(EclKW.fseek_grdecl(file, "PERMX"))
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMY"))
        file.close()

        file = copen(self.src_file, "r")
        kw1 = EclKW.read_grdecl(file, "PERMX")
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMX"))
        self.assertTrue(EclKW.fseek_grdecl(file, "PERMX", rewind=True))
        file.close()


    def test_fseek2(self):
        test_src = self.createTestPath("local/ECLIPSE/grdecl-test/test.grdecl")
        # Test kw at the the very start
        file = copen(test_src, "r")
        self.assertTrue(EclKW.fseek_grdecl(file, "PERMX"))

        # Test commented out kw:
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMY"))
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMZ"))

        # Test ignore not start of line:
        self.assertTrue(EclKW.fseek_grdecl(file, "MARKER"))
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMXYZ"))

        # Test rewind
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMX", rewind=False))
        self.assertTrue(EclKW.fseek_grdecl(file, "PERMX", rewind=True))

        # Test multiline comments + blanks
        self.assertTrue(EclKW.fseek_grdecl(file, "LASTKW"))


    def test_fseek_dos(self):
        test_src = self.createTestPath("local/ECLIPSE/grdecl-test/test.grdecl_dos")  # File formatted with \r\n line endings.
        # Test kw at the the very start
        file = copen(test_src, "r")
        self.assertTrue(EclKW.fseek_grdecl(file, "PERMX"))

        # Test commented out kw:
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMY"))
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMZ"))

        # Test ignore not start of line:
        self.assertTrue(EclKW.fseek_grdecl(file, "MARKER"))
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMXYZ"))

        # Test rewind
        self.assertFalse(EclKW.fseek_grdecl(file, "PERMX", rewind=False))
        self.assertTrue(EclKW.fseek_grdecl(file, "PERMX", rewind=True))

        # Test multiline comments + blanks
        self.assertTrue(EclKW.fseek_grdecl(file, "LASTKW"))
