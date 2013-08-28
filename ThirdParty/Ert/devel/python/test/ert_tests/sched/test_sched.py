#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'test_sched.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os
from ert.sched import SchedFile
from ert_tests import ExtendedTestCase




class SchedFileTest(ExtendedTestCase):
    def setUp(self):
        src_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/target.SCH")
        self.start_time = datetime.date(2000, 1, 1)

        self.sched_file = SchedFile(src_file, self.start_time)
        self.file_list = []

    def addFile( self, filename ):
        self.file_list.append(filename)

    def tearDown(self):
        for f in self.file_list:
            if os.path.exists(f):
                os.unlink(f)

    def test_load(self):
        self.assertTrue(self.sched_file, "Load failed")


    def test_length(self):
        self.assertEqual(self.sched_file.length, 63)


    def test_write_loop(self):
        self.sched_file.write("/tmp/schedule1", 62)
        sched_file2 = SchedFile("/tmp/schedule1", self.start_time)
        sched_file2.write("/tmp/schedule2", 62)
        self.assertFilesAreEqual("/tmp/schedule1", "/tmp/schedule2")

        self.addFile("/tmp/schedule1")
        self.addFile("/tmp/schedule2")
