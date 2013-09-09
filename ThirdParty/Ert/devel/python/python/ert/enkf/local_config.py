#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'local_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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


class LocalConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_config_files(self):
        """ @rtype: StringList """
        return LocalConfig.cNamespace().get_config_files(self).setParent(self)

    def clear_config_files(self):
        LocalConfig.cNamespace().clear_config_files(self)

    def add_config_file(self, filename):
        LocalConfig.cNamespace().add_config_file(self, filename)

    def free(self):
        LocalConfig.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("local_config", LocalConfig)
cwrapper.registerType("local_config_obj", LocalConfig.createPythonObject)
cwrapper.registerType("local_config_ref", LocalConfig.createCReference)

LocalConfig.cNamespace().free = cwrapper.prototype("void local_config_free( local_config )")
LocalConfig.cNamespace().get_config_files = cwrapper.prototype("stringlist_ref local_config_get_config_files( local_config )")
LocalConfig.cNamespace().clear_config_files = cwrapper.prototype("void local_config_clear_config_files( local_config )")
LocalConfig.cNamespace().add_config_file = cwrapper.prototype("void local_config_add_config_file( local_config , char*)")

