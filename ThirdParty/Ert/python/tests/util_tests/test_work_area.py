#!/usr/bin/env python
#  Copyright (C) 2014  Equinor ASA, Norway.
#
#  The file 'test_work_area.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

from ecl.util.test import TestAreaContext
from tests import EclTest


class WorkAreaTest(EclTest):

    def test_full_path(self):
        with TestAreaContext("TestArea") as test_area:
            with open("test_file" , "w") as fileH:
                fileH.write("Something")

                self.assertTrue( os.path.isfile( "test_file") )

            with self.assertRaises(IOError):
                full_path = test_area.getFullPath( "does/not/exists" )

            with self.assertRaises(IOError):
                full_path = test_area.getFullPath( "/already/absolute" )

            full_path = test_area.getFullPath( "test_file" )
            self.assertTrue( os.path.isfile( full_path ))
            self.assertTrue( os.path.isabs( full_path ))


    def test_IOError(self):
        with TestAreaContext("TestArea") as test_area:
            with self.assertRaises(IOError):
                test_area.copy_file( "Does/not/exist" )

            with self.assertRaises(IOError):
                test_area.install_file( "Does/not/exist" )

            with self.assertRaises(IOError):
                test_area.copy_directory( "Does/not/exist" )

            with self.assertRaises(IOError):
                test_area.copy_parent_directory( "Does/not/exist" )

            os.makedirs("path1/path2")
            with open("path1/file.txt" , "w") as f:
                f.write("File ...")

            with self.assertRaises(IOError):
                test_area.copy_directory( "path1/file.txt" )


    def test_multiple_areas(self):
        original_dir = os.getcwd()
        context_dirs = []
        for i in range(3):
            loop_dir = os.getcwd()
            self.assertEqual(loop_dir, original_dir,
                    'Wrong folder before creating TestAreaContext. Loop: {} -- CWD: {} '
                    .format(i, loop_dir))

            with TestAreaContext("test_multiple_areas") as t:
                t_dir = t.get_cwd()

                self.assertNotIn(t_dir, context_dirs,
                        'Multiple TestAreaContext objects in the same folder. Loop {} -- CWD: {}'
                        .format(i, loop_dir))
                context_dirs.append(t_dir)

                # It is possible to make the following assert fail, but whoever run the tests should
                # try really really hard to make that happen
                self.assertNotEqual(t_dir, original_dir,
                        'TestAreaContext in the current working directory. Loop: {} -- CWD: {}'
                        .format(i, loop_dir))

            loop_dir = os.getcwd()
            self.assertEqual(loop_dir, original_dir,
                    'Wrong folder after creating TestAreaContext. Loop: {} -- CWD: {} '
                            .format(i, loop_dir))

