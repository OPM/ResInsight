#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'test_ert_gui_import.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.test.extended_testcase import ExtendedTestCase
from ert_tests.import_tester import ImportTester


class ErtGuiImportTest(ExtendedTestCase):

    def test_ert_gui(self):
        module = __import__("ert_gui")
        path = os.path.abspath(module.__file__)
        path = os.path.dirname(path)

        self.assertTrue(ImportTester.importRecursively(path, "ert_gui"))
