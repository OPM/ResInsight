# Copyright (C) 2017  Statoil ASA, Norway.
#
# This file is part of ERT - Ensemble based Reservoir Tool.
#
# ERT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ERT is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
# for more details.
from os.path import abspath

from ert.ecl import EclGrid
from ert.enkf.config import FieldTypeEnum, FieldConfig
from ert.enkf.enums import EnkfFieldFileFormatEnum
from ert.test import ExtendedTestCase, TestAreaContext

class FieldConfigTest(ExtendedTestCase):

    def test_field_guess_filetype(self):
        with TestAreaContext('field_config') as test_context:
            fname = abspath('test.kw.grdecl')
            print(fname)
            with open(fname, 'w') as f:
                f.write("-- my comment\n")
                f.write("-- more comments\n")
                f.write("SOWCR\n")
                for i in range(256//8): # technicalities demand file has >= 256B
                    f.write("0 0 0 0\n")

            ft = FieldConfig.guessFiletype(fname)
            grdecl_type = EnkfFieldFileFormatEnum(5)
            self.assertEqual('ECL_GRDECL_FILE', grdecl_type.name)
            self.assertEqual(grdecl_type, ft)

    def test_field_type_enum(self):
        self.assertEqual(FieldTypeEnum(2), FieldTypeEnum.ECLIPSE_PARAMETER)
        gen = FieldTypeEnum.GENERAL
        self.assertEqual('GENERAL', str(gen))
        gen = FieldTypeEnum(3)
        self.assertEqual('GENERAL', str(gen))

    def test_export_format(self):
        self.assertEqual(FieldConfig.exportFormat("file.grdecl"),     EnkfFieldFileFormatEnum.ECL_GRDECL_FILE)
        self.assertEqual(FieldConfig.exportFormat("file.xyz.grdecl"), EnkfFieldFileFormatEnum.ECL_GRDECL_FILE)
        self.assertEqual(FieldConfig.exportFormat("file.roFF"),       EnkfFieldFileFormatEnum.RMS_ROFF_FILE)
        self.assertEqual(FieldConfig.exportFormat("file.xyz.roFF"),   EnkfFieldFileFormatEnum.RMS_ROFF_FILE)

        with self.assertRaises(ValueError):
            FieldConfig.exportFormat("file.xyz")

        with self.assertRaises(ValueError):
            FieldConfig.exportFormat("file.xyz")

    def test_basics(self):
        grid = EclGrid.createRectangular((17,13,11),(1,1,1))
        fc = FieldConfig('PORO',grid)
        print(fc)
        print(str(fc))
        print(repr(fc))
        pfx = 'FieldConfig(type'
        rep = repr(fc)
        self.assertEqual(pfx, rep[:len(pfx)])
        fc_xyz = fc.get_nx(),fc.get_ny(),fc.get_nz()
        ex_xyz = 17,13,11
        self.assertEqual(ex_xyz, fc_xyz)
        self.assertEqual(0,     fc.get_truncation_mode())
        self.assertEqual(ex_xyz, (grid.getNX(), grid.getNY(), grid.getNZ()))
