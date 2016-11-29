#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'test_analysis_module.py' is part of ERT - Ensemble based Reservoir Tool.
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

import ert
from ert.test import ExtendedTestCase
from ert.analysis import AnalysisModule, AnalysisModuleLoadStatusEnum, AnalysisModuleOptionsEnum

from ert.util.enums import RngAlgTypeEnum, RngInitModeEnum
from ert.util.rng import RandomNumberGenerator


class AnalysisModuleTest(ExtendedTestCase):
    def setUp(self):
        self.libname = ert.ert_lib_path + "/rml_enkf.so"
        self.rng = RandomNumberGenerator(RngAlgTypeEnum.MZRAN, RngInitModeEnum.INIT_DEFAULT)

    def createAnalysisModule(self):
        return AnalysisModule(self.rng, lib_name = self.libname)

    def test_load_status_enum(self):
        source_file_path = "libanalysis/include/ert/analysis/analysis_module.h"
        self.assertEnumIsFullyDefined(AnalysisModuleLoadStatusEnum, "analysis_module_load_status_enum", source_file_path)

    def test_analysis_module(self):
        am = self.createAnalysisModule()

        self.assertEqual(am.getLibName(), self.libname)

        self.assertFalse(am.getInternal())

        self.assertTrue(am.setVar("ITER", "1"))

        self.assertEqual(am.getTableName(), "analysis_table")

        self.assertTrue(am.checkOption(AnalysisModuleOptionsEnum.ANALYSIS_ITERABLE))

        self.assertTrue(am.hasVar("ITER"))

        self.assertIsInstance(am.getDouble("ENKF_TRUNCATION"), float)

        self.assertIsInstance(am.getInt("ITER"), int)

    def test_set_get_var(self):
        mod = AnalysisModule( self.rng , name = "STD_ENKF" )
        with self.assertRaises(KeyError):
            mod.setVar("NO-NOT_THIS_KEY" , 100)


        with self.assertRaises(KeyError):
            mod.getInt("NO-NOT_THIS_KEY")





    def test_create_internal(self):
        with self.assertRaises( KeyError ):
            mod = AnalysisModule( self.rng , name = "STD_ENKFXXX" )

        mod = AnalysisModule( self.rng , name = "STD_ENKF" )


