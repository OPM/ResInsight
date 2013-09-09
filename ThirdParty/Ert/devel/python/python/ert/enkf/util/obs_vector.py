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


class ObsVector(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_state_kw(self):
        """ @rtype: str """
        return ObsVector.cNamespace().get_state_kw(self)

    def iget_node(self, index):
        ObsVector.cNamespace().iget_node(self, index) #warn: should return something!

    def get_num_active(self):
        """ @rtype: int """
        return ObsVector.cNamespace().get_num_active(self)

    def iget_active(self, index):
        """ @rtype: bool """
        return ObsVector.cNamespace().iget_active(self, index)

    def free(self):
        ObsVector.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("obs_vector", ObsVector)
cwrapper.registerType("obs_vector_obj", ObsVector.createPythonObject)
cwrapper.registerType("obs_vector_ref", ObsVector.createCReference)

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.

ObsVector.cNamespace().free = cwrapper.prototype("void obs_vector_free( obs_vector )")
ObsVector.cNamespace().get_state_kw = cwrapper.prototype("char* obs_vector_get_state_kw( obs_vector )")
ObsVector.cNamespace().iget_node = cwrapper.prototype("c_void_p obs_vector_iget_node( obs_vector, int)")
ObsVector.cNamespace().get_num_active = cwrapper.prototype("int obs_vector_get_num_active( obs_vector )")
ObsVector.cNamespace().iget_active = cwrapper.prototype("bool obs_vector_iget_active( obs_vector, int)")
