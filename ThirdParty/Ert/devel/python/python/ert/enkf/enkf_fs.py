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

import  ctypes
from ctypes import c_buffer
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
from    enkf_enum             import *
from ert.ert.enums import enkf_var_type, ert_state_enum
from ert.enkf.time_map import TimeMap
from    ert.util.buffer        import Buffer
import  libenkf
from ert.ert.c_enums import state_enum
from ert.ert.c_enums import var_type
class EnkfFs(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.close )

    def has_node(self, node_key, var_type, report_step, iens, state):
        return cfunc.has_node(self, node_key, var_type, report_step, iens, state)

    def has_vector(self, node_key, var_type, iens, state):
        return cfunc.has_vector(self, node_key, var_type, iens, state)        

    
    def fread_node(self, key, type, step, member, value):
        buffer = Buffer(100)
        cfunc.fread_node(self, buffer, key, type, step, member, value)

    def fread_vector(self, key, type, member, value):
        buffer = Buffer(100)
        cfunc.fread_vector(self, buffer, key, type, member, value)

    @property
    def get_time_map(self):
        return TimeMap(cfunc.get_time_map(self), parent = self)

    @staticmethod
    def exists(path):
        return cfunc.exists(path)
    
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "enkf_fs" , EnkfFs )

cfunc = CWrapperNameSpace("enkf_fs")

cfunc.close               = cwrapper.prototype("void enkf_fs_close(enkf_fs)")
cfunc.has_node            = cwrapper.prototype("bool enkf_fs_has_node(enkf_fs, char*, c_uint, int, int, c_uint)")
cfunc.has_vector          = cwrapper.prototype("bool enkf_fs_has_vector(enkf_fs, char*, c_uint, int, c_uint)")
cfunc.fread_node          = cwrapper.prototype("void enkf_fs_fread_node(enkf_fs, buffer, char*, c_uint, int, int, c_uint)")
cfunc.fread_vector        = cwrapper.prototype("void enkf_fs_fread_vector(enkf_fs, buffer, char*, c_uint, int, c_uint)")
cfunc.get_time_map        = cwrapper.prototype("c_void_p enkf_fs_get_time_map(enkf_fs)")
cfunc.exists              = cwrapper.prototype("bool enkf_fs_exists(char*)")
