#  Copyright (C) 2011  Statoil ASA, Norway.
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

import os
import getpass

try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

import time
import shutil
from ert.ecl import EclSum
from ert.job_queue import QueueDriverEnum, RSHDriver
from ert.test import ExtendedTestCase , TestAreaContext

path = "Statoil/ECLIPSE/Gurbat"

base = "ECLIPSE_SHORT"
LSF_base = "ECLIPSE_SHORT_MPI"

case = "%s/%s" % (path, base)
LSF_case = "%s/%s" % (path, LSF_base)



class EclSubmitTest(ExtendedTestCase):
    nfs_work_path = None
    rsh_servers = None

    def setUp(self):
        if hasattr(self, "argv"):
            if len(self.argv) > 0:
                self.nfs_work_path = self.argv[0]

            if len(self.argv) > 1:
                self.rsh_servers = self.argv[1]

    def make_run_path(self, iens, LSF=False):
        run_path = "run%d" % iens
        if os.path.exists(run_path):
            shutil.rmtree(run_path)

        os.makedirs(run_path)
        shutil.copytree("%s/include" % self.createTestPath(path), "%s/include" % run_path)
        if LSF:
            shutil.copy("%s.DATA" % self.createTestPath(LSF_case), run_path)
        else:
            shutil.copy("%s.DATA" % self.createTestPath(case), run_path)

        return os.path.abspath(run_path)
