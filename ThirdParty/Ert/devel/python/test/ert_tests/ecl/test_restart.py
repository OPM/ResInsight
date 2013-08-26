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
from _ctypes import ArgumentError
import os
import datetime
from ert.ecl import EclFile
from ert_tests import ExtendedTestCase





class RestartTest(ExtendedTestCase):
    def setUp(self):
        self.xfile0 = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0000")
        self.u_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        self.fmt_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.FUNRST")
        self.grid_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")
        self.file_list = []

    def addFile( self, filename ):
        self.file_list.append(filename)

    def tearDown(self):
        for f in self.file_list:
            if os.path.exists(f):
                os.unlink(f)


    def report_file_test(self, fname):
        self.assertTrue(EclFile.contains_report_step(fname, 4))
        self.assertTrue(EclFile.contains_report_step(fname, 0))
        self.assertTrue(EclFile.contains_report_step(fname, 62))
        self.assertFalse(EclFile.contains_report_step(fname, -1))
        self.assertFalse(EclFile.contains_report_step(fname, 100))

        f = EclFile(fname)
        self.assertTrue(f.has_report_step(4))
        self.assertTrue(f.has_report_step(0))
        self.assertTrue(f.has_report_step(62))

        self.assertFalse(f.has_report_step(-1))
        self.assertFalse(f.has_report_step(100))


    def test_report(self):
        self.report_file_test(self.u_file)


    def test_date(self):
        f = EclFile(self.u_file)
        self.assertTrue(f.has_sim_time(datetime.datetime(2001, 6, 1)))
        self.assertFalse(f.has_sim_time(datetime.datetime(2005, 6, 1)))
        self.assertFalse(f.has_sim_time(datetime.datetime(1999, 6, 1)))
        self.assertFalse(f.has_sim_time(datetime.datetime(2001, 6, 11)))

        self.assertTrue(EclFile.contains_sim_time(self.u_file, datetime.datetime(2001, 6, 1)))
        self.assertFalse(EclFile.contains_sim_time(self.u_file, datetime.datetime(2005, 6, 1)))
        self.assertFalse(EclFile.contains_sim_time(self.u_file, datetime.datetime(1999, 6, 1)))
        self.assertFalse(EclFile.contains_sim_time(self.u_file, datetime.datetime(2001, 6, 11)))


    def report_list_file_test(self, fname, rlist0):
        rlist = EclFile.file_report_list(fname)
        self.assertAlmostEqualList(rlist, rlist0)

        f = EclFile(fname)
        rlist = f.report_list
        self.assertAlmostEqualList(rlist, rlist0)


    def test_report_list(self):
        rlist0 = range(63)
        self.report_list_file_test(self.u_file, rlist0)

        rlist0 = [0]
        self.report_list_file_test(self.xfile0, rlist0)

        f = EclFile(self.grid_file)
        with self.assertRaises(ArgumentError): #argumentError wraps the expected TypeError
            EclFile.file_report_list(f)


    def test_dates(self):
        f = EclFile(self.u_file)
        dates = f.dates
        self.assertTrue(len(dates) == 63)

        f = EclFile(self.xfile0)
        dates = f.dates
        self.assertTrue(len(dates) == 1)
        self.assertTrue(dates[0] == datetime.datetime(2000, 1, 1))


    def test_name(self):
        f = EclFile(self.u_file)
        self.assertTrue(f.name == self.u_file)

        f = EclFile(self.xfile0)
        self.assertTrue(f.name == self.xfile0)


    def test_kw( self ):
        f = EclFile(self.u_file)
        kw1 = f["SWAT"][40]
        kw2 = f.restart_get_kw("SWAT", datetime.datetime(2003, 3, 1))
        kw3 = f.restart_get_kw("SWAT", datetime.datetime(2003, 3, 1), copy=True)

        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal(kw3))

        kw4 = f.restart_get_kw("SWAT", datetime.datetime(2009, 3, 1))
        self.assertIsNone(kw4)

