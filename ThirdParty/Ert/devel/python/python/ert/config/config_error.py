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
from ert.config import ConfigPrototype
from ert.cwrap import BaseCClass



class ConfigError(BaseCClass):
    TYPE_NAME = "config_error"
    _free  = ConfigPrototype("void config_error_free(config_error)")
    _count = ConfigPrototype("int config_error_count(config_error)")
    _iget  = ConfigPrototype("char* config_error_iget(config_error, int)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __getitem__(self, index):
        """ @rtype: str """
        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if index >= size:
            raise IndexError("Index out of range: %d < %d" % (index, size))

        return self._iget(index)

    def __len__(self):
        """ @rtype: int """
        return self._count()

    def free(self):
        self._free()

