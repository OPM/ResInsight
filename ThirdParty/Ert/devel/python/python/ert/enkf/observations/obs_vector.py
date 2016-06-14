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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.config import EnkfConfigNode
from ert.enkf.enums import EnkfObservationImplementationType
from ert.enkf.observations import BlockObservation, SummaryObservation, GenObservation


class ObsVector(BaseCClass):
    def __init__(self, observation_type, observation_key, config_node, num_reports):
        """
        @type observation_type: EnkfObservationImplementationType
        @type observation_key: str
        @type config_node: EnkfConfigNode
        @type num_reports: int
        """
        assert isinstance(observation_type, EnkfObservationImplementationType)
        assert isinstance(observation_key, str)
        assert isinstance(config_node, EnkfConfigNode)
        assert isinstance(num_reports, int)
        pointer = ObsVector.cNamespace().alloc(observation_type, observation_key, config_node, num_reports)
        super(ObsVector, self).__init__(pointer)


    def getDataKey(self):
        """ @rtype: str """
        return ObsVector.cNamespace().get_state_kw(self)

    def getObservationKey(self):
        """ @rtype: str """
        return ObsVector.cNamespace().get_observation_key(self)


    def getNode(self, index):
        """ @rtype: SummaryObservation or BlockObservation or GenObservation"""

        pointer = ObsVector.cNamespace().iget_node(self, index)

        node_type = self.getImplementationType()
        if node_type == EnkfObservationImplementationType.SUMMARY_OBS:
            return SummaryObservation.createCReference(pointer, self)
        elif node_type == EnkfObservationImplementationType.BLOCK_OBS:
            return BlockObservation.createCReference(pointer, self)
        elif node_type == EnkfObservationImplementationType.GEN_OBS:
            return GenObservation.createCReference(pointer, self)
        else:
            raise AssertionError("Node type '%s' currently not supported!" % node_type)

    
    def __iter__(self):
        """ Iterate over active report steps; return node"""
        cur = -1
        run = True
        for step in self.getStepList():
            yield self.getNode( step )
            

        
    def getStepList(self):
        """
        Will return an IntVector with the active report steps.
        """
        return ObsVector.cNamespace().get_step_list(self)

    def activeStep(self):
        """Assuming the observation is only active for one report step, this
        method will return that report step - if it is active for more
        than one report step the method will raise an exception.
        """
        step_list = self.getStepList()
        if len(step_list):
            return step_list[0]
        else:
            raise ValueError("The activeStep() method can *ONLY* be called for obervations with one active step")
            

    def getActiveCount(self):
        """ @rtype: int """
        return ObsVector.cNamespace().get_num_active(self)

    def isActive(self, index):
        """ @rtype: bool """
        return ObsVector.cNamespace().iget_active(self, index)

    def getNextActiveStep(self, previous_step=-1):
        """ @rtype: int """
        return ObsVector.cNamespace().get_next_active_step(self, previous_step)

    def getImplementationType(self):
        """ @rtype: EnkfObservationImplementationType """
        return ObsVector.cNamespace().get_impl_type(self)

    def installNode(self, index, node):
        assert isinstance(node, SummaryObservation)
        node.convertToCReference(self)
        ObsVector.cNamespace().install_node(self, index, node.from_param(node))

    def getConfigNode(self):
        """ @rtype: EnkfConfigNode """
        return ObsVector.cNamespace().get_config_node(self).setParent(self)

    
    def createLocalObs(self):
        """
        Will create a LocalObsDataNode instance with all timesteps set.
        """
        return ObsVector.cNamespace().create_local_node( self )

    
    def hasData(self, active_mask, fs):
        """ @rtype: bool """
        return ObsVector.cNamespace().has_data(self, active_mask, fs)

    def free(self):
        ObsVector.cNamespace().free(self)

    def getTotalChi2(self, fs, realization_number):
        """ @rtype: float """
        return ObsVector.cNamespace().get_total_chi2(self, fs, realization_number)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("obs_vector", ObsVector)

ObsVector.cNamespace().alloc = cwrapper.prototype("c_void_p obs_vector_alloc(enkf_obs_impl_type, char*, enkf_config_node, int)")
ObsVector.cNamespace().free = cwrapper.prototype("void obs_vector_free( obs_vector )")
ObsVector.cNamespace().get_state_kw = cwrapper.prototype("char* obs_vector_get_state_kw( obs_vector )")
ObsVector.cNamespace().get_observation_key = cwrapper.prototype("char* obs_vector_get_key( obs_vector )")
ObsVector.cNamespace().iget_node = cwrapper.prototype("c_void_p obs_vector_iget_node( obs_vector, int)")
ObsVector.cNamespace().get_num_active = cwrapper.prototype("int obs_vector_get_num_active( obs_vector )")
ObsVector.cNamespace().iget_active = cwrapper.prototype("bool obs_vector_iget_active( obs_vector, int)")
ObsVector.cNamespace().get_impl_type = cwrapper.prototype("enkf_obs_impl_type obs_vector_get_impl_type( obs_vector)")
ObsVector.cNamespace().install_node = cwrapper.prototype("void obs_vector_install_node(obs_vector, int, c_void_p)")
ObsVector.cNamespace().get_next_active_step = cwrapper.prototype("int obs_vector_get_next_active_step(obs_vector, int)")
ObsVector.cNamespace().has_data = cwrapper.prototype("bool obs_vector_has_data(obs_vector , bool_vector , enkf_fs)")
ObsVector.cNamespace().get_config_node = cwrapper.prototype("enkf_config_node_ref obs_vector_get_config_node(obs_vector)")
ObsVector.cNamespace().get_total_chi2 = cwrapper.prototype("double obs_vector_total_chi2(obs_vector, enkf_fs, int)")
ObsVector.cNamespace().get_obs_key = cwrapper.prototype("char* obs_vector_get_obs_key(obs_vector)")
ObsVector.cNamespace().get_step_list = cwrapper.prototype("int_vector_ref obs_vector_get_step_list(obs_vector)")
ObsVector.cNamespace().create_local_node = cwrapper.prototype("local_obsdata_node_obj obs_vector_alloc_local_node(obs_vector)")
