#  Copyright (C) 2018  Statoil ASA, Norway.
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

import os.path
import subprocess
from subprocess import CalledProcessError as CallError

from ecl.grid import Cell, EclGrid
from ecl.summary import EclSum
from tests import EclTest
from ecl.util.test.ecl_mock import createEclSum
from ecl.util.test import TestAreaContext


def fopr(days):
    return days

def fopt(days):
    return days

def fgpt(days):
    if days < 50:
        return days
    else:
        return 100 - days

def create_case(num_mini_step = 10, case = "CSV"):
    length = 100
    return createEclSum(case, [("FOPT", None , 0) , ("FOPR" , None , 0), ("FGPT" , None , 0)],
                        sim_length_days = length,
                        num_report_step = 10,
                        num_mini_step = num_mini_step,
                        func_table = {"FOPT" : fopt,
                                      "FOPR" : fopr ,
                                      "FGPT" : fgpt })



class SummaryResampleTest(EclTest):

    @classmethod
    def setUpClass(cls):
        cls.script = os.path.join(cls.SOURCE_ROOT, "bin/summary_resample")
        cls.case = create_case()

    def test_run_default(self):
        with TestAreaContext(""):
            self.case.fwrite()

            # Too few arguments
            with self.assertRaises(CallError):
                subprocess.check_call([self.script])

            # Too few arguments
            with self.assertRaises(CallError):
                subprocess.check_call([self.script, "CSV"])

            # Invalid first arguments
            with self.assertRaises(CallError):
                subprocess.check_call([self.script, "DOES_NOT_EXIST", "OUTPUT"])

            # Should run OK:
            subprocess.check_call([self.script, "CSV", "OUTPUT"])
            output_case = EclSum("OUTPUT")
            self.assertEqual( output_case.get_data_start_time(), self.case.get_data_start_time())
            self.assertEqual( output_case.get_end_time(), self.case.get_end_time())

            with self.assertRaises(CallError):
                subprocess.check_call([self.script, "CSV", "OUTPUT", "--refcase=does/not/exist"])

            refcase = create_case( num_mini_step = 7, case = "REFCASE")
            refcase.fwrite()
            subprocess.check_call([self.script, "CSV", "OUTPUT", "--refcase=REFCASE"])
            output_case = EclSum("OUTPUT")
            self.assertEqual( output_case.get_data_start_time(), refcase.get_data_start_time())
            self.assertEqual( output_case.get_end_time(), refcase.get_end_time())
            time_points = output_case.alloc_time_vector(False)
            t1 = output_case.alloc_time_vector(False)
            t2 = refcase.alloc_time_vector(False)
            self.assertEqual(t1,t2)
