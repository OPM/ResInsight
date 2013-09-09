#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_kw_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.util import StringList


class GenKwConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_template_file(self):
        return GenKwConfig.cNamespace().get_template_file(self)

    def get_parameter_file(self):
        return GenKwConfig.cNamespace().get_parameter_file(self)

    def alloc_name_list(self):
        return GenKwConfig.cNamespace().alloc_name_list(self).setParent(self)

    def free(self):
        GenKwConfig.cNamespace().free(self)

##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("gen_kw_config", GenKwConfig)
cwrapper.registerType("gen_kw_config_obj", GenKwConfig.createPythonObject)
cwrapper.registerType("gen_kw_config_ref", GenKwConfig.createCReference)

GenKwConfig.cNamespace().free = cwrapper.prototype("void gen_kw_config_free( gen_kw_config )")
GenKwConfig.cNamespace().get_template_file = cwrapper.prototype("char* gen_kw_config_get_template_file(gen_kw_config)")
GenKwConfig.cNamespace().get_parameter_file = cwrapper.prototype("char* gen_kw_config_get_parameter_file(gen_kw_config)")
GenKwConfig.cNamespace().alloc_name_list = cwrapper.prototype("stringlist_ref gen_kw_config_alloc_name_list(gen_kw_config)")
