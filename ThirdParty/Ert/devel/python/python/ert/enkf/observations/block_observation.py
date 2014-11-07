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
from ert.enkf import ENKF_LIB, NodeId


class BlockObservation(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getCoordinate(self, index):
        """ @rtype: tuple of (int, int, int) """
        i = BlockObservation.cNamespace().iget_i(self, index)
        j = BlockObservation.cNamespace().iget_j(self, index)
        k = BlockObservation.cNamespace().iget_k(self, index)
        return i, j, k

    def __len__(self):
        """ @rtype: int """
        return BlockObservation.cNamespace().get_size(self)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield cur
            cur += 1

    def getValue(self, index):
        """ @rtype: float """
        return BlockObservation.cNamespace().get_value(self, index)

    def getStd(self, index):
        """ @rtype: float """
        return BlockObservation.cNamespace().get_std(self, index)

    def getDepth(self, index):
        """ @rtype: float """
        return BlockObservation.cNamespace().get_depth(self, index)

    def getData(self, state, obs_index, node_id):
        """
        @type state: c_void_p
        @type obs_index: int
        @type node_id: NodeId
        @rtype: float """

        return BlockObservation.cNamespace().iget_data(self, state, obs_index, node_id)


    def free(self):
        BlockObservation.cNamespace().free(self)

##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("block_obs", BlockObservation)

BlockObservation.cNamespace().free = cwrapper.prototype("void block_obs_free( block_obs )")
BlockObservation.cNamespace().iget_i = cwrapper.prototype("int block_obs_iget_i(block_obs, int)")
BlockObservation.cNamespace().iget_j = cwrapper.prototype("int block_obs_iget_j( block_obs, int)")
BlockObservation.cNamespace().iget_k = cwrapper.prototype("int block_obs_iget_k( block_obs , int)")
BlockObservation.cNamespace().get_size = cwrapper.prototype("int block_obs_get_size( block_obs )")
BlockObservation.cNamespace().get_std = cwrapper.prototype("double block_obs_iget_std( block_obs, int )")
BlockObservation.cNamespace().get_value = cwrapper.prototype("double block_obs_iget_value( block_obs, int)")
BlockObservation.cNamespace().get_depth = cwrapper.prototype("double block_obs_iget_depth( block_obs, int)")

BlockObservation.cNamespace().iget_data = cwrapper.prototype("double block_obs_iget_data(block_obs, c_void_p, int, node_id)")

