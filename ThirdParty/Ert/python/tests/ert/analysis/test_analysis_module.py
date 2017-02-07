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

from ert.util import Matrix

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


    def test_initX_enkf_linalg_lowrankCinv(self):
        """Test AnalysisModule.initX with EE=False and GE=False"""
        mod = AnalysisModule( self.rng , name = "STD_ENKF" )
        A, S, R, dObs, E, D = self._n_identity_mcs()
        self.assertFalse(mod.getBool('USE_EE'))
        self.assertFalse(mod.getBool('USE_GE'))

        elt_a, elt_b = 1.222, -0.111
        vals = (elt_a, elt_b, elt_b,
                elt_b, elt_a, elt_b,
                elt_b, elt_b, elt_a)
        expected = self.construct_matrix(3, vals)

        X = mod.initX(A, S, R, dObs, E, D)
        self._matrix_close(X, expected)

    def test_initX_enkf_linalg_lowrank_EE(self):
        """Test AnalysisModule.initX with EE=True and GE=False"""
        mod = AnalysisModule( self.rng , name = "STD_ENKF" )
        A, S, R, dObs, E, D = self._n_identity_mcs()
        mod.setVar('USE_EE', True)
        self.assertTrue(mod.getBool('USE_EE'))
        self.assertFalse(mod.getBool('USE_GE'))

        elt_a, elt_b = 1.33, -0.167
        vals = (elt_a, elt_b, elt_b,
                elt_b, elt_a, elt_b,
                elt_b, elt_b, elt_a)
        expected = self.construct_matrix(3,  vals)
        X = mod.initX(A, S, R, dObs, E, D)
        self._matrix_close(X, expected)

    def test_initX_subspace_inversion_algorithm(self):
        """Test AnalysisModule.initX with EE=True and GE=True, the subspace inversion algorithm"""
        mod = AnalysisModule( self.rng , name = "STD_ENKF" )
        A, S, R, dObs, E, D = self._n_identity_mcs()

        mod.setVar('USE_EE', True)
        mod.setVar('USE_GE', True)
        self.assertTrue(mod.getBool('USE_EE'))
        self.assertTrue(mod.getBool('USE_GE'))

        vals = ( 1.39,  -0.111, -0.278,
                -0.111,  1.39,  -0.278,
                 -0.278, -0.278,  1.56 )
        expected = self.construct_matrix(3, vals)
        X = mod.initX(A, S, R, dObs, E, D)
        self._matrix_close(X, expected)

    def construct_matrix(self, n, vals):
        """Constructs n*n matrix with vals as entries"""
        self.assertEqual(n*n, len(vals))
        m = Matrix(n,n)
        idx = 0
        for i in range(n):
            for j in range(n):
                m[(i,j)] = vals[idx]
                idx += 1
        return m

    def _n_identity_mcs(self, n=6,s=3):
        """return n copies of the identity matrix on s*s elts"""
        return tuple([Matrix.identity(s) for i in range(n)])

    def _matrix_close(self, m1, m2, epsilon=0.01):
        """Check that matrices m1 and m2 are of same dimension and that they are
        pointwise within epsilon difference."""

        c = m1.columns()
        r = m1.rows()
        self.assertEqual(c, m2.columns(), 'Number of columns for m1 differ from m2')
        self.assertEqual(r, m2.rows(), 'Number of rows for m1 differ from m2')
        for i in range(0, c):
            for j in range(0,r):
                pos  = (i,j)
                diff = abs(m1[pos] - m2[pos])
                self.assertTrue(diff <= epsilon, 'Matrices differ at (i,j) = (%d,%d). %f != %f' % (i, j, m1[pos], m2[pos]))
