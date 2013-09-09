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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
import ert


class EnkfNode(BaseCClass):
    def __init__(self, config_node):
        assert isinstance(config_node, ert.enkf.data.EnkfConfigNode)

        c_pointer = EnkfNode.cNamespace().alloc(config_node)
        super(EnkfNode, self).__init__(c_pointer, config_node, True)

    def user_get(self, fs, key, report_step, iens, state, value):
        return EnkfNode.cNamespace().user_get(self, fs, key, report_step, iens, state, value)

    def user_get_vector( self, fs, key, iens, state, vector):
        return EnkfNode.cNamespace().user_get_vector(self, fs, key, iens, state, vector)

    def value_ptr(self):
        EnkfNode.cNamespace().value_ptr(self)

    def vector_storage(self):
        return EnkfNode.cNamespace().vector_storage(self)

    def free(self):
        EnkfNode.cNamespace().free(self)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_node", EnkfNode)
cwrapper.registerType("enkf_node_obj", EnkfNode.createPythonObject)
cwrapper.registerType("enkf_node_ref", EnkfNode.createCReference)

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.


EnkfNode.cNamespace().free = cwrapper.prototype("void enkf_node_free( enkf_node )")
EnkfNode.cNamespace().alloc = cwrapper.prototype("c_void_p enkf_node_alloc( enkf_node)")
EnkfNode.cNamespace().user_get = cwrapper.prototype("bool enkf_node_user_get_no_id(enkf_node , enkf_fs , char*  , int, int , c_uint, double*)")
EnkfNode.cNamespace().user_get_vector = cwrapper.prototype("bool enkf_node_user_get_vector( enkf_node , enkf_fs , char*, int, c_uint, double_vector)")
EnkfNode.cNamespace().value_ptr = cwrapper.prototype("void enkf_node_value_ptr(enkf_node)")
EnkfNode.cNamespace().vector_storage = cwrapper.prototype("bool enkf_node_vector_storage(enkf_node)")
