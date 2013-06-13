#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'obs_vector.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class ObsVector(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def get_state_kw(self):
        return cfunc.get_state_kw(self)
    
    def iget_node(self, index):
        cfunc.iget_node(self,index)

    @property
    def get_num_active(self):
        return cfunc.get_num_active(self)

    @property
    def iget_active(self, index):
        return cfunc.iget_active(self, index)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "obs_vector" , ObsVector )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("obs_vector")


cfunc.free                = cwrapper.prototype("void obs_vector_free( obs_vector )")
cfunc.get_state_kw        = cwrapper.prototype("char* obs_vector_get_state_kw( obs_vector )")
cfunc.iget_node           = cwrapper.prototype("void obs_vector_iget_node( obs_vector, int)")
cfunc.get_num_active      = cwrapper.prototype("int obs_vector_get_num_active( obs_vector )")
cfunc.iget_active         = cwrapper.prototype("bool obs_vector_iget_active( obs_vector, int)")
