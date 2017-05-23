#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'log.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from __future__ import print_function
from cwrap import BaseCClass
from ecl.util import UtilPrototype


class Log(BaseCClass):
    _get_filename = UtilPrototype("char* log_get_filename(log)")
    _reopen = UtilPrototype("void log_reopen(log, char*)")
    _get_level = UtilPrototype("int log_get_level(log)")
    _set_level = UtilPrototype("void log_set_level(log, int)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_filename(self):
        return self._get_filename()
        # return "ert_config.log"

    def reopen(self, filename):
        print('Logfile cannot be reopened')
        # cfunc.reopen( self , filename)

    def get_level(self):
        return self._get_level()

    def set_level(self, level):
        pass
        # self._set_level(self, level)
