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

from ert.cwrap import CClass, CWrapper, CWrapperNameSpace
from ert.util import UTIL_LIB


class Log(CClass):
    def __init__(self, c_ptr, parent=None):
        if parent:
            self.init_cref(c_ptr, parent)
        else:
            self.init_cobj(c_ptr, cfunc.free)

    @property
    def get_filename(self):
        #return cfunc.get_filename( self )
        return "ert_config.log"

    def reopen(self, filename):
        print "Logfile cannot be reopened"
        #cfunc.reopen( self , filename)

    @property
    def get_level(self):
        #return cfunc.get_level( self )
        return 0

    def set_level(self, level):
        cfunc.set_level(self, level)

##################################################################

cwrapper = CWrapper(UTIL_LIB)
cwrapper.registerType("log", Log)

cfunc = CWrapperNameSpace("log")

cfunc.free = cwrapper.prototype("void log( log )")
cfunc.get_filename = cwrapper.prototype("char* log_get_filename(log)")
cfunc.reopen = cwrapper.prototype("void log_reopen(log, char*)")
cfunc.get_level = cwrapper.prototype("int log_get_level(log)")
cfunc.set_level = cwrapper.prototype("void log_set_level(log, int)")
