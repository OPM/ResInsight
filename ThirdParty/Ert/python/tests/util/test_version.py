#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'test_version.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os.path

import ert
from ert.test import ExtendedTestCase
from ert.util import Version


class VersionTest(ExtendedTestCase):
    def setUp(self):
        pass

    def test_create(self):
        v1 = Version(1, 8, 6)
        self.assertFalse(v1.isDevelVersion())

        self.assertEqual(v1.versionString(), "1.8.6")
        self.assertEqual(v1.versionTuple(), (1, 8, 6))

        v2 = Version(2, 0, "X")
        self.assertTrue(v2.isDevelVersion())

    def test_eq(self):
        v1 = Version(1, 2, 3)
        v2 = Version(1, 2, 3)

        self.assertTrue(v1 == v2)
        self.assertEqual(v1, v2)
        self.assertEqual(str(v1), str(v2))
        self.assertEqual(repr(v1), repr(v2))
        self.assertFalse(v1 != v2)

        v1 = Version(1, 2, "X")
        v2 = Version(1, 2, "Y")
        self.assertTrue(v1 != v2)
        self.assertFalse(v1 == v2)

        v1 = Version(1, 2, "X")
        v2 = Version(1, 2, 0)
        self.assertTrue(v1 != v2)
        self.assertFalse(v1 == v2)

        v1 = Version(1, 2, "X")
        v2 = Version(1, 3, "X")
        self.assertTrue(v1 != v2)
        self.assertFalse(v1 == v2)

        v1 = Version(1, 2, "X")
        v2 = (1, 3, "X")
        self.assertTrue(v1 != v2)
        self.assertFalse(v1 == v2)

    def test_ge(self):
        v1 = Version(1, 2, 3)
        v2 = Version(1, 2, 3)
        v3 = (1, 2, 2)

        self.assertEqual(str(v1), str(v2))
        self.assertEqual(repr(v1), repr(v2))

        self.assertTrue(v1 >= v2)
        self.assertFalse(v1 < v2)

        self.assertTrue(v1 >= v3)
        self.assertFalse(v1 < v3)

        v1 = Version(1, 2, "X")
        v2 = Version(1, 1, 9)
        self.assertTrue(v1 > v2)

        v2 = Version(1, 2, "X")
        self.assertTrue(v1 >= v2)

        v2 = Version(1, 2, 0)
        self.assertFalse(v1 >= v2)

        self.assertNotEqual(str(v1), str(v2))
        self.assertNotEqual(repr(v1), repr(v2))


    def test_current(self):
        current = Version.currentVersion()
        self.assertTrue(current > (0, 0, 0))
        pfx = 'Version(major='
        self.assertEqual(pfx, repr(current)[:len(pfx)])

    def test_import(self):
        from ert import Version as globalVersion
        v1 = globalVersion(1, 1, 2)
        v2 = Version(1, 1, 2)

        self.assertTrue(v1 == v2)
        self.assertEqual(v1, v2)
        self.assertEqual(repr(v1), repr(v2))


    def test_root_version(self):
        cv = Version.currentVersion( )
        self.assertEqual( ert.__version__ , cv.versionString() )


    def test_root_path(self):
        self.assertTrue( os.path.isdir( os.path.join( ert.root() , "ert")))
        self.assertTrue( os.path.isfile( os.path.join( ert.root() , "ert", "__init__.py")))
        
