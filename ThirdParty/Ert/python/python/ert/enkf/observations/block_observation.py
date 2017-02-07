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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf import NodeId, FieldConfig
from ert.enkf.observations import BlockDataConfig


class BlockObservation(BaseCClass):
    TYPE_NAME = "block_obs"

    _alloc              = EnkfPrototype("void*  block_obs_alloc( char* , block_data_config , ecl_grid )", bind = False)
    _free               = EnkfPrototype("void   block_obs_free( block_obs )")
    _iget_i             = EnkfPrototype("int    block_obs_iget_i(block_obs, int)")
    _iget_j             = EnkfPrototype("int    block_obs_iget_j( block_obs, int)")
    _iget_k             = EnkfPrototype("int    block_obs_iget_k( block_obs , int)")
    _get_size           = EnkfPrototype("int    block_obs_get_size( block_obs )")
    _get_std            = EnkfPrototype("double block_obs_iget_std( block_obs, int )")
    _get_std_scaling    = EnkfPrototype("double block_obs_iget_std_scaling( block_obs, int )")
    _update_std_scaling = EnkfPrototype("void   block_obs_update_std_scale(block_obs , double , active_list)")
    _get_value          = EnkfPrototype("double block_obs_iget_value( block_obs, int)")
    _get_depth          = EnkfPrototype("double block_obs_iget_depth( block_obs, int)")
    _add_field_point    = EnkfPrototype("void   block_obs_append_field_obs( block_obs, int,int,int,double,double)")
    _add_summary_point  = EnkfPrototype("void   block_obs_append_summary_obs( block_obs, int, int, int, double, double)")
    _iget_data          = EnkfPrototype("double block_obs_iget_data(block_obs, void*, int, node_id)")



    def __init__(self , obs_key , data_config , grid):
        c_ptr = self._alloc( obs_key , data_config , grid )
        super(BlockObservation, self).__init__(c_ptr)
        
        
    def getCoordinate(self, index):
        """ @rtype: tuple of (int, int, int) """
        i = self._iget_i(index)
        j = self._iget_j(index)
        k = self._iget_k(index)
        return i, j, k

    def __len__(self):
        """ @rtype: int """
        return self._get_size()

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield cur
            cur += 1

    def addPoint(self , i,j,k , value , std , sum_key = None):
        if sum_key is None:
            self._add_field_point(i,j,k,value,std)
        else:
            self._add_summary_point(i,j,k,sum_key,value,std)
            

    def getValue(self, index):
        """ @rtype: float """
        return self._get_value(index)

    def getStd(self, index):
        """ @rtype: float """
        return self._get_std(index)

    def getStdScaling(self , index):
        """ @rtype: float """
        return self._get_std_scaling(index)

    def updateStdScaling(self , factor , active_list):
        self._update_std_scaling(factor , active_list)
    
    
    def getDepth(self, index):
        """ @rtype: float """
        return self._get_depth(index)

    def getData(self, state, obs_index, node_id):
        """
        @type state: c_void_p
        @type obs_index: int
        @type node_id: NodeId
        @rtype: float """

        return self._iget_data(state, obs_index, node_id)


    def free(self):
        self._free()

    def __repr__(self):
        return 'BlockObservation(size = %d) at 0x%x' % (len(self), self._address())
