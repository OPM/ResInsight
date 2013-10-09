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
from ert.enkf.data import EnkfConfigNode
from ert.enkf.enums import EnkfObservationImplementationType
from ert.enkf.observations.summary_observation import SummaryObservation



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

    def get_state_kw(self):
        """ @rtype: str """
        return ObsVector.cNamespace().get_state_kw(self)

    def getNode(self, index):
        pointer = ObsVector.cNamespace().iget_node(self, index)

        node_type = self.getImplementationType()
        if node_type == EnkfObservationImplementationType.SUMMARY_OBS:
            return SummaryObservation.createCReference(pointer, self)
        else:
            raise AssertionError("Node type '%s' currently not supported!" % node_type)

    def getActiveCount(self):
        """ @rtype: int """
        return ObsVector.cNamespace().get_num_active(self)

    def isActive(self, index):
        """ @rtype: bool """
        return ObsVector.cNamespace().iget_active(self, index)

    def getImplementationType(self):
        """ @rtype: EnkfObservationImplementationType """
        return ObsVector.cNamespace().get_impl_type(self)

    def installNode(self, index, node):
        assert isinstance(node, SummaryObservation)
        node.convertToCReference(self)
        ObsVector.cNamespace().install_node(self, index, node.from_param(node))

    def free(self):
        ObsVector.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("obs_vector", ObsVector)
cwrapper.registerType("obs_vector_obj", ObsVector.createPythonObject)
cwrapper.registerType("obs_vector_ref", ObsVector.createCReference)

ObsVector.cNamespace().alloc = cwrapper.prototype("c_void_p obs_vector_alloc(enkf_obs_impl_type, char*, enkf_config_node, int)")
ObsVector.cNamespace().free = cwrapper.prototype("void obs_vector_free( obs_vector )")
ObsVector.cNamespace().get_state_kw = cwrapper.prototype("char* obs_vector_get_state_kw( obs_vector )")
ObsVector.cNamespace().iget_node = cwrapper.prototype("c_void_p obs_vector_iget_node( obs_vector, int)")
ObsVector.cNamespace().get_num_active = cwrapper.prototype("int obs_vector_get_num_active( obs_vector )")
ObsVector.cNamespace().iget_active = cwrapper.prototype("bool obs_vector_iget_active( obs_vector, int)")
ObsVector.cNamespace().get_impl_type = cwrapper.prototype("enkf_obs_impl_type obs_vector_get_impl_type( obs_vector)")
ObsVector.cNamespace().install_node = cwrapper.prototype("void obs_vector_install_node(obs_vector, int, c_void_p)")
