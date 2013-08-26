#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_node.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from    ert.enkf.enkf_fs import EnkfFs
from    ert.util.tvector import DoubleVector
import  libenkf
class EnkfNode(CClass):

    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )   
            
    @staticmethod
    def alloc(config_node):
        node = EnkfNode(c_ptr = cfunc.alloc( config_node ) , parent = config_node)
        return node 

    def user_get(self, fs, key, report_step, iens, state, value):
        return cfunc.user_get(self, fs, key, report_step, iens, state, value)

    def user_get_vector( self , fs , key , iens , state , vector):
        return cfunc.user_get_vector( self , fs , key , iens , state, vector)

    def value_ptr(self):
        cfunc.value_ptr(self)

    @property
    def vector_storage(self):
        return cfunc.vector_storage(self)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "enkf_node" , EnkfNode )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("enkf_node")


cfunc.free                = cwrapper.prototype("void enkf_node_free( enkf_node )")
cfunc.alloc               = cwrapper.prototype("c_void_p enkf_node_alloc( enkf_node)")
cfunc.user_get            = cwrapper.prototype("bool enkf_node_user_get_no_id(enkf_node , enkf_fs , char*  , int, int , c_uint, double*)")
cfunc.user_get_vector     = cwrapper.prototype("bool enkf_node_user_get_vector( enkf_node , enkf_fs , char*, int, c_uint, double_vector)")
cfunc.value_ptr           = cwrapper.prototype("void enkf_node_value_ptr(enkf_node)")
cfunc.vector_storage      = cwrapper.prototype("bool enkf_node_vector_storage(enkf_node)")
