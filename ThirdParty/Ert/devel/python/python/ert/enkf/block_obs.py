#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'block_obs.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class BlockObs(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )

    def iget_i(self , index):
        return cfunc.iget_i( self ,index )

    def iget_j(self , index):
        return cfunc.iget_j( self ,index )

    def iget_k(self , index):
        return cfunc.iget_k( self ,index )

    def get_size(self):
        return cfunc.get_size(self)

    def iget(self, index, value, std):
        return cfunc.iget(self, index, value, std)

##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "block_obs" , BlockObs )
cfunc = CWrapperNameSpace("block_obs")


cfunc.free                = cwrapper.prototype("void block_obs_free( block_obs )")
cfunc.iget_i              = cwrapper.prototype("int block_obs_iget_i(block_obs, int)")
cfunc.iget_j              = cwrapper.prototype("int block_obs_iget_j( block_obs, int)")
cfunc.iget_k              = cwrapper.prototype("int block_obs_iget_k( block_obs , int)")
cfunc.get_size            = cwrapper.prototype("int block_obs_get_size( block_obs )")
cfunc.iget                = cwrapper.prototype("void block_obs_iget( block_obs, int, double*, double*)")
