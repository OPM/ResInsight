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
from ert.enkf import ENKF_LIB, EnkfFs, NodeId
from ert.enkf.data import EnkfConfigNode , GenKw , GenData
from ert.enkf.data.gen_data_config import GenDataConfig
from ert.enkf.enums import EnkfStateType
from ert.enkf.enums.ert_impl_type_enum import ErtImplType


class EnkfNode(BaseCClass):
    def __init__(self, config_node, private=False):
        assert isinstance(config_node, EnkfConfigNode)

        if private:
            c_pointer = EnkfNode.cNamespace().alloc_private(config_node)
        else:
            c_pointer = EnkfNode.cNamespace().alloc(config_node)

        super(EnkfNode, self).__init__(c_pointer, config_node, True)

    # def user_get(self, fs, key, report_step, iens, state, value):
    #     return EnkfNode.cNamespace().user_get(self, fs, key, report_step, iens, state, value)
    #
    # def user_get_vector( self, fs, key, iens, state, vector):
    #     return EnkfNode.cNamespace().user_get_vector(self, fs, key, iens, state, vector)


    def valuePointer(self):
        return EnkfNode.cNamespace().value_ptr(self)

    def asGenData(self):
        """ @rtype: GenData """
        impl_type = EnkfNode.cNamespace().get_impl_type(self)
        assert impl_type == ErtImplType.GEN_DATA

        return GenData.createCReference(self.valuePointer(), self)


    def asGenKw(self):
        impl_type = EnkfNode.cNamespace().get_impl_type(self)
        assert impl_type == ErtImplType.GEN_KW

        return GenKw.createCReference(self.valuePointer(), self)



    # def vector_storage(self):
    #     return EnkfNode.cNamespace().vector_storage(self)

    # def getConfig(self):
    #     """ @rtype: EnkfConfigNode """
    #     #todo: fix this!!!! wrong return type in prototype!
    #     return EnkfNode.cNamespace().get_config(self).setParent(self)


    def tryLoad(self, fs, node_id):
        """
        @type fs: EnkfFS
        @type node_id: NodeId
        @rtype: bool
        """
        assert isinstance(fs, EnkfFs)
        assert isinstance(node_id, NodeId)

        return EnkfNode.cNamespace().try_load(self, fs, node_id)

    def save(self , fs , node_id ):
        EnkfNode.cNamespace().store(self , fs , True , node_id)


    def free(self):
        EnkfNode.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("enkf_node", EnkfNode)

EnkfNode.cNamespace().free = cwrapper.prototype("void enkf_node_free( enkf_node )")
EnkfNode.cNamespace().alloc = cwrapper.prototype("c_void_p enkf_node_alloc(enkf_node)")
EnkfNode.cNamespace().alloc_private = cwrapper.prototype("c_void_p enkf_node_alloc_private_container(enkf_node)")

# EnkfNode.cNamespace().user_get = cwrapper.prototype("bool enkf_node_user_get_no_id(enkf_node , enkf_fs , char*  , int, int , c_uint, double*)")
# EnkfNode.cNamespace().user_get_vector = cwrapper.prototype("bool enkf_node_user_get_vector( enkf_node , enkf_fs , char*, int, c_uint, double_vector)")
EnkfNode.cNamespace().value_ptr = cwrapper.prototype("c_void_p enkf_node_value_ptr(enkf_node)")
# EnkfNode.cNamespace().vector_storage = cwrapper.prototype("bool enkf_node_vector_storage(enkf_node)")

EnkfNode.cNamespace().try_load = cwrapper.prototype("bool enkf_node_try_load(enkf_node, enkf_fs, node_id)")
EnkfNode.cNamespace().get_impl_type = cwrapper.prototype("ert_impl_type_enum enkf_node_get_impl_type(enkf_node)")
EnkfNode.cNamespace().store = cwrapper.prototype("void enkf_node_store(enkf_node, enkf_fs , bool , node_id)")
#todo fix this
# EnkfNode.cNamespace().get_config = cwrapper.prototype("c_void_p enkf_node_get_config(enkf_node)")
