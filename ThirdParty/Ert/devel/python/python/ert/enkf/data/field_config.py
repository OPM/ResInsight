#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'field_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB


class FieldConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_type(self):
        return FieldConfig.cNamespace().get_type(self)

    def get_truncation_mode(self):
        return FieldConfig.cNamespace().get_truncation_mode(self)

    def get_truncation_min(self):
        return FieldConfig.cNamespace().get_truncation_min(self)

    def get_init_transform_name(self):
        return FieldConfig.cNamespace().get_init_transform_name(self)

    def get_output_transform_name(self):
        return FieldConfig.cNamespace().get_output_transform_name(self)

    def get_truncation_max(self):
        return FieldConfig.cNamespace().get_truncation_max(self)

    def get_nx(self):
        return FieldConfig.cNamespace().get_nx(self)

    def get_ny(self):
        return FieldConfig.cNamespace().get_ny(self)

    def get_nz(self):
        return FieldConfig.cNamespace().get_nz(self)

    def ijk_active(self, i, j, k):
        return FieldConfig.cNamespace().ijk_active(self, i, j, k)

    def free(self):
        FieldConfig.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("field_config", FieldConfig)
cwrapper.registerType("field_config_obj", FieldConfig.createPythonObject)
cwrapper.registerType("field_config_ref", FieldConfig.createCReference)

FieldConfig.cNamespace().free = cwrapper.prototype("void field_config_free( field_config )")
FieldConfig.cNamespace().get_type = cwrapper.prototype("int field_config_get_type(field_config)")
FieldConfig.cNamespace().get_truncation_mode = cwrapper.prototype("int field_config_get_truncation_mode(field_config)")
FieldConfig.cNamespace().get_truncation_min = cwrapper.prototype("double field_config_get_truncation_min(field_config)")
FieldConfig.cNamespace().get_truncation_max = cwrapper.prototype("double field_config_get_truncation_max(field_config)")
FieldConfig.cNamespace().get_init_transform_name = cwrapper.prototype("char* field_config_get_init_transform_name(field_config)")
FieldConfig.cNamespace().get_output_transform_name = cwrapper.prototype("char* field_config_get_output_transform_name(field_config)")
FieldConfig.cNamespace().ijk_active = cwrapper.prototype("bool field_config_ijk_active(field_config, int, int, int)")
FieldConfig.cNamespace().get_nx = cwrapper.prototype("int field_config_get_nx(field_config)")
FieldConfig.cNamespace().get_ny = cwrapper.prototype("int field_config_get_ny(field_config)")
FieldConfig.cNamespace().get_nz = cwrapper.prototype("int field_config_get_nz(field_config)")
FieldConfig.cNamespace().get_grid = cwrapper.prototype("c_void_p field_config_get_grid(field_config)")  #todo: fix return type
