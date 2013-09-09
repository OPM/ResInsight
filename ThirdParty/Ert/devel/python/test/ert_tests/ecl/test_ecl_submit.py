# #!/usr/bin/env python
# #  Copyright (C) 2011  Statoil ASA, Norway.
# #
# #  The file 'sum_test.py' is part of ERT - Ensemble based Reservoir Tool.
# #
# #  ERT is free software: you can redistribute it and/or modify
# #  it under the terms of the GNU General Public License as published by
# #  the Free Software Foundation, either version 3 of the License, or
# #  (at your option) any later version.
# #
# #  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
# #  WARRANTY; without even the implied warranty of MERCHANTABILITY or
# #  FITNESS FOR A PARTICULAR PURPOSE.
# #
# #  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
# #  for more details.
#
import os
import getpass
from unittest2 import skipIf
import time
import shutil
from ert.ecl import EclQueue, EclSum
from ert.job_queue import QueueDriverEnum, RSHDriver
from ert.util.test_area import TestAreaContext
from ert_tests import ExtendedTestCase


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


class LSFSubmitTest(EclSubmitTest):

    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow LSF job submit skipped!")
    def test_start_parameters(self):
        self.assertIsNotNone(self.nfs_work_path, "NFS work path missing!")
        self.assertIsNone(self.rsh_servers)


    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow LSF job submit skipped!")
    def test_LSF_submit(self):
        root = os.path.join(self.nfs_work_path, getpass.getuser(), "ert-test/python/ecl_submit/LSF")
        if not os.path.exists(root):
            os.makedirs(root)
        os.chdir(root)

        num_submit = 6
        queue = EclQueue(driver_type=QueueDriverEnum.LSF_DRIVER, max_running=4, size=num_submit)
        path_list = []

        for iens in (range(num_submit)):
            run_path = self.make_run_path(iens, LSF=True)
            path_list.append(run_path)
            job = queue.submitDataFile("%s/%s.DATA" % (run_path, LSF_base))

        while queue.isRunning():
            time.sleep(1)

        for path in path_list:
            sum = EclSum("%s/%s" % (path, LSF_base))
            self.assertIsInstance(sum, EclSum)
            self.assertEqual(2, sum.last_report)


class RSHSubmitTest(EclSubmitTest):
    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow RSH job submit skipped!")
    def test_start_parameters(self):
        self.assertIsNotNone(self.nfs_work_path, "NFS work path missing!")
        self.assertIsNotNone(self.rsh_servers, "RSH servers missing!")

    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow RSH job submit skipped!")
    def test_RSH_submit(self):
        root = os.path.join(self.nfs_work_path, getpass.getuser(), "ert-test/python/ecl_submit/RSH")
        if not os.path.exists(root):
            os.makedirs(root)
        os.chdir(root)

        num_submit = 6
        host_list = []
        for h in self.rsh_servers.split():
            tmp = h.split(":")
            if len(tmp) > 1:
                num = int(tmp[1])
            else:
                num = 1
            host_list.append((tmp[0], num))

        queue = EclQueue(RSHDriver(3, host_list), size=num_submit)
        path_list = []

        for iens in (range(num_submit)):
            run_path = self.make_run_path(iens)
            path_list.append(run_path)
            job = queue.submitDataFile("%s/%s.DATA" % (run_path, base))

        while queue.isRunning():
            time.sleep(1)

        for path in path_list:
            sum = EclSum("%s/%s" % (path, base))
            self.assertIsInstance(sum, EclSum)
            self.assertEqual(2, sum.last_report)

class LocalSubmitTest(EclSubmitTest):

    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow LOCAL job submit skipped!")
    def test_LOCAL_submit(self):
        #work_area = TestArea("python/ecl_submit/LOCAL", True)

        with TestAreaContext("python/ecl_submit/LOCAL", True) as work_area:
            num_submit = 4
            queue = EclQueue(driver_type=QueueDriverEnum.LOCAL_DRIVER, max_running=2)
            path_list = []

            for iens in range(num_submit):
                run_path = self.make_run_path(iens)
                path_list.append(run_path)
                job = queue.submitDataFile("%s/%s.DATA" % (run_path, base))

            queue.submit_complete()
            while queue.isRunning():
                time.sleep(1)

            for path in path_list:
                sum = EclSum("%s/%s" % (path, base))
                self.assertIsInstance(sum, EclSum)
                self.assertEqual(2, sum.last_report)
                shutil.rmtree(path)

