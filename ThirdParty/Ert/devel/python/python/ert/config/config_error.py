#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_fs.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.config import CONFIG_LIB
from ert.cwrap import BaseCClass, CWrapper



class ConfigError(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __getitem__(self, index):
        """ @rtype: str """
        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if index >= size:
            raise IndexError("Index out of range: %d < %d" % (index, size))

        return ConfigError.cNamespace().iget(self, index)

    def __len__(self):
        """ @rtype: int """
        return ConfigError.cNamespace().count(self)

    def free(self):
        ConfigError.cNamespace().free(self)

##################################################################

cwrapper = CWrapper(CONFIG_LIB)
cwrapper.registerObjectType("config_error", ConfigError)

ConfigError.cNamespace().free = cwrapper.prototype("void config_error_free(config_error)")
ConfigError.cNamespace().count = cwrapper.prototype("int config_error_count(config_error)")
ConfigError.cNamespace().iget = cwrapper.prototype("char* config_error_iget(config_error, int)")


