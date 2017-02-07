# Copyright (C) 2012  Statoil ASA, Norway.
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
from cwrap import BaseCClass

from ert.enkf import EnkfPrototype
from ert.enkf.enums import EnkfFieldFileFormatEnum
from ert.ecl import EclGrid
from .field_type_enum import FieldTypeEnum

class FieldConfig(BaseCClass):
    TYPE_NAME = "field_config"

    _alloc                     = EnkfPrototype("void*  field_config_alloc_empty(char* , ecl_grid , void* , bool)", bind = False)
    _free                      = EnkfPrototype("void   field_config_free( field_config )")
    _get_type                  = EnkfPrototype("field_type_enum field_config_get_type(field_config)")
    _get_truncation_mode       = EnkfPrototype("int    field_config_get_truncation_mode(field_config)")
    _get_truncation_min        = EnkfPrototype("double field_config_get_truncation_min(field_config)")
    _get_truncation_max        = EnkfPrototype("double field_config_get_truncation_max(field_config)")
    _get_init_transform_name   = EnkfPrototype("char*  field_config_get_init_transform_name(field_config)")
    _get_output_transform_name = EnkfPrototype("char*  field_config_get_output_transform_name(field_config)")
    _ijk_active                = EnkfPrototype("bool   field_config_ijk_active(field_config, int, int, int)")
    _get_nx                    = EnkfPrototype("int    field_config_get_nx(field_config)")
    _get_ny                    = EnkfPrototype("int    field_config_get_ny(field_config)")
    _get_nz                    = EnkfPrototype("int    field_config_get_nz(field_config)")
    _get_grid                  = EnkfPrototype("ecl_grid_ref field_config_get_grid(field_config)")
    _export_format             = EnkfPrototype("enkf_field_file_format_enum field_config_default_export_format(char*)", bind = False)
    _guess_filetype            = EnkfPrototype("enkf_field_file_format_enum field_config_guess_file_type(char*)", bind = False)

    def __init__(self , kw , grid):
        c_ptr = self._alloc( kw , grid , None , False )
        super(FieldConfig, self).__init__(c_ptr)

    @classmethod
    def exportFormat(cls , filename):
        export_format = cls._export_format( filename )
        if export_format in [ EnkfFieldFileFormatEnum.ECL_GRDECL_FILE , EnkfFieldFileFormatEnum.RMS_ROFF_FILE ]:
            return export_format
        else:
            raise ValueError("Could not determine grdecl / roff format from:%s" % filename)

    @classmethod
    def guessFiletype(cls, filename):
        return cls._guess_filetype(filename)

    def get_type(self):
        return self._get_type()

    def get_truncation_mode(self):
        return self._get_truncation_mode()

    def get_truncation_min(self):
        return self._get_truncation_min()

    def get_init_transform_name(self):
        return self._get_init_transform_name()

    def get_output_transform_name(self):
        return self._get_output_transform_name()

    def get_truncation_max(self):
        return self._get_truncation_max()

    def get_nx(self):
        return self._get_nx()

    def get_ny(self):
        return self._get_ny()

    def get_nz(self):
        return self._get_nz()

    def get_grid(self):
        return self._get_grid()

    def ijk_active(self, i, j, k):
        return self._ijk_active(i, j, k)

    def free(self):
        self._free()

    def __repr__(self):
        tp = self.get_type()
        nx,ny,nz = self.get_nx(),self.get_ny(),self.get_nz()
        cnt = 'type = %s, nx = %d, ny = %d, nz = %d' % (tp, nx,ny,nz)
        return self._create_repr(cnt)
