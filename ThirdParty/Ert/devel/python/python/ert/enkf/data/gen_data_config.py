#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_data_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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


class GenDataConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_template_file(self):
        return GenDataConfig.cNamespace().get_template_file(self)

    def get_template_key(self):
        return GenDataConfig.cNamespace().get_template_key(self)

    def get_initial_size(self):
        return GenDataConfig.cNamespace().get_initial_size(self)

    def get_output_format(self):
        return GenDataConfig.cNamespace().get_output_format(self)

    def get_input_format(self):
        return GenDataConfig.cNamespace().get_input_format(self)

    def free(self):
        GenDataConfig.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("gen_data_config", GenDataConfig)
cwrapper.registerType("gen_data_config_obj", GenDataConfig.createPythonObject)
cwrapper.registerType("gen_data_config_ref", GenDataConfig.createCReference)

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.

GenDataConfig.cNamespace().free = cwrapper.prototype("void gen_data_config_free( gen_data_config )")
GenDataConfig.cNamespace().get_output_format = cwrapper.prototype("c_void_p gen_data_config_get_output_format(gen_data_config)") #todo: fix return type
GenDataConfig.cNamespace().get_input_format = cwrapper.prototype("c_void_p gen_data_config_get_input_format(gen_data_config)") #todo: fix return type
GenDataConfig.cNamespace().get_template_file = cwrapper.prototype("char* gen_data_config_get_template_file(gen_data_config)")
GenDataConfig.cNamespace().get_template_key = cwrapper.prototype("char* gen_data_config_get_template_key(gen_data_config)")
GenDataConfig.cNamespace().get_initial_size = cwrapper.prototype("int gen_data_config_get_initial_size(gen_data_config)")

