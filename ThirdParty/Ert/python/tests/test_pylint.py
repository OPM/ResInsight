#  Copyright (C) 2017 Statoil ASA, Norway.
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

from ecl.test import LintTestCase

class LintErt(LintTestCase):
    """Tests that no file in ert needs linting"""

    #  Temporarily disabled due to monkey patching camel case
    # def test_lint_ecl(self):
    #     white = ['ecl_kw.py', 'ecl_type.py', 'ecl_sum.py', 'ecl_grid.py', 'ecl_npv.py']  # TODO fix issues and remove
    #     self.assertLinted('ecl/ecl', whitelist=white)

    def test_lint_geo(self):
        self.assertLinted('ecl/geo')

    def test_lint_util(self):
        self.assertLinted('ecl/util')

    def test_lint_well(self):
        self.assertLinted('ecl/well')
