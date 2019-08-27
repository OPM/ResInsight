#!/usr/bin/env python
#  Copyright (C) 2017 Equinor ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

import sys
import fnmatch
import os
import unittest
try:
    from pylint import epylint as lint
except ImportError:
    sys.stderr.write("Could not import pylint module - lint based testing will be skipped\n")
    lint = None


class LintTestCase(unittest.TestCase):
    """This class is a test case for linting."""

    LINT_ARGS = ['-d', 'R,C,W'] + \
                ['--extension-pkg-whitelist=numpy']


    @staticmethod
    def _get_lintable_files(paths, whitelist=()):
        """Recursively traverses all folders in paths for *.py files"""
        matches = []
        for folder in paths:
            for root, _, filenames in os.walk(folder):
                for filename in fnmatch.filter(filenames, '*.py'):
                    if filename not in whitelist:
                        matches.append(os.path.join(root, filename))
        return matches


    def assertLinted(self, paths, whitelist=()):  # noqa
        """Takes a path to a folder or a list of paths to folders and recursively finds
        all *.py files within that folder except the ones with filenames in whitelist.

        Will assert lint.lint(fname) == 0 for every *.py file found.
        """
        if lint is None:
            self.skipTest("pylint not installed")

        if isinstance(paths, str):
            paths = [paths]
        files = self._get_lintable_files(paths, whitelist=whitelist)
        for f in files:
            self.assertEqual(0, lint.lint(f, self.LINT_ARGS), 'Linting required for %s' % f)
