#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'test_active_list.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.enkf import ActiveList
from ert.enkf import ActiveMode
from ert.test import ExtendedTestCase



class ActiveListTest(ExtendedTestCase):

    def test_active_mode_enum(self):
        self.assertEqual(ActiveMode.ALL_ACTIVE,    1)
        self.assertEqual(ActiveMode.INACTIVE,      2)
        self.assertEqual(ActiveMode.PARTLY_ACTIVE, 3)
        self.assertEqual(ActiveMode(1).name, 'ALL_ACTIVE')
        self.assertEqual(ActiveMode(2).name, 'INACTIVE')
        self.assertEqual(ActiveMode(3).name, 'PARTLY_ACTIVE')

    def test_active_size(self):
        al = ActiveList()
        self.assertEqual(None, al.getActiveSize(None))
        self.assertEqual(7,    al.getActiveSize(7))
        self.assertEqual(-1,   al.getActiveSize(-1))

        al.addActiveIndex( 10 )
        self.assertEqual(1, al.getActiveSize(7))
        al.addActiveIndex( 10 )
        self.assertEqual(1, al.getActiveSize(7))
        al.addActiveIndex( 100 )
        self.assertEqual(2, al.getActiveSize(7))

    def test_create(self):
        active_list = ActiveList()
        self.assertEqual( active_list.getMode() , ActiveMode.ALL_ACTIVE )
        active_list.addActiveIndex( 10 )
        self.assertEqual( active_list.getMode() , ActiveMode.PARTLY_ACTIVE )

    def test_repr(self):
        al = ActiveList()
        rep = repr(al)
        self.assertFalse('PARTLY_ACTIVE' in rep)
        self.assertFalse('INACTIVE' in rep)
        self.assertTrue('ALL_ACTIVE' in rep)
        pfx = 'ActiveList('
        self.assertEqual(pfx, rep[:len(pfx)])
        for i in range(150):
            al.addActiveIndex( 3*i )
        rep = repr(al)
        self.assertTrue('150' in rep)
        self.assertTrue('PARTLY_ACTIVE' in rep)
        self.assertFalse('INACTIVE' in rep)
        self.assertFalse('ALL_ACTIVE' in rep)
