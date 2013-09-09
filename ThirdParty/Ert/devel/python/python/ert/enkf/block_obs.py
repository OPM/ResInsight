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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB


class BlockObs(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def iget_i(self, index):
        return BlockObs.cNamespace().iget_i(self, index)

    def iget_j(self, index):
        return BlockObs.cNamespace().iget_j(self, index)

    def iget_k(self, index):
        return BlockObs.cNamespace().iget_k(self, index)

    def get_size(self):
        return BlockObs.cNamespace().get_size(self)

    def iget(self, index, value, std):
        return BlockObs.cNamespace().iget(self, index, value, std)

    def free(self):
        BlockObs.cNamespace().free(self)

##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("block_obs", BlockObs)
cwrapper.registerType("block_obs_obj", BlockObs.createPythonObject)
cwrapper.registerType("block_obs_ref", BlockObs.createCReference)

BlockObs.cNamespace().free = cwrapper.prototype("void block_obs_free( block_obs )")
BlockObs.cNamespace().iget_i = cwrapper.prototype("int block_obs_iget_i(block_obs, int)")
BlockObs.cNamespace().iget_j = cwrapper.prototype("int block_obs_iget_j( block_obs, int)")
BlockObs.cNamespace().iget_k = cwrapper.prototype("int block_obs_iget_k( block_obs , int)")
BlockObs.cNamespace().get_size = cwrapper.prototype("int block_obs_get_size( block_obs )")
BlockObs.cNamespace().iget = cwrapper.prototype("void block_obs_iget( block_obs, int, double*, double*)")
