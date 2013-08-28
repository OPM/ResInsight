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

import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
from    enkf_enum             import *
import  libenkf
from ert.util.stringlist import StringList
class LocalConfig(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def get_config_files(self):
        config_files = StringList(c_ptr = cfunc.get_config_files(self), parent = self)
        return config_files

    def clear_config_files(self):
        cfunc.clear_config_files(self)

    def add_config_file(self, file):
        cfunc.add_config_file(self, file)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "local_config" , LocalConfig )

cfunc = CWrapperNameSpace("local_config")

##################################################################
##################################################################

cfunc.free               = cwrapper.prototype("void local_config_free( local_config )")
cfunc.get_config_files   = cwrapper.prototype("c_void_p local_config_get_config_files( local_config )")
cfunc.clear_config_files = cwrapper.prototype("void local_config_clear_config_files( local_config )")
cfunc.add_config_file    = cwrapper.prototype("void local_config_add_config_file( local_config , char*)")

