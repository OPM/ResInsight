#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'test_enkf.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import RunArg
from ert.enkf.enums import EnkfRunType
from ert.test import ExtendedTestCase



class RunArgTest(ExtendedTestCase):

    def test_create(self):
        run_arg = RunArg.ENSEMBLE_EXPERIMENT(fs , 10 , 10 , "/path/to/run")
        
