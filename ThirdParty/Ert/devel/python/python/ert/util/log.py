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

from ert.cwrap import CWrapper, BaseCClass
from ert.util import UTIL_LIB


class Log(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_filename(self):
        return Log.cNamespace().get_filename( self )
        # return "ert_config.log"

    def reopen(self, filename):
        print "Logfile cannot be reopened"
        #cfunc.reopen( self , filename)

    def get_level(self):
        return Log.cNamespace().get_level( self )

    def set_level(self, level):
        pass
        # Log.cNamespace().set_level(self, level)

##################################################################

cwrapper = CWrapper(UTIL_LIB)
cwrapper.registerType("log", Log)
cwrapper.registerType("log_obj", Log.createPythonObject)
cwrapper.registerType("log_ref", Log.createCReference)


Log.cNamespace().get_filename = cwrapper.prototype("char* log_get_filename(log)")
Log.cNamespace().reopen = cwrapper.prototype("void log_reopen(log, char*)")
Log.cNamespace().get_level = cwrapper.prototype("int log_get_level(log)")
Log.cNamespace().set_level = cwrapper.prototype("void log_set_level(log, int)")
