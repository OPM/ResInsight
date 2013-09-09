#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'enkf_config_node.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf.data import FieldConfig, GenDataConfig, GenKwConfig, EnkfNode, SummaryConfig


class EnkfConfigNode(BaseCClass):
    FIELD = 104
    GEN_KW = 107
    SUMMARY = 110
    GEN_DATA = 113

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getImplementationType( self ):
        return EnkfConfigNode.cNamespace().get_impl_type(self)

    def get_var_type( self ):
        return EnkfConfigNode.cNamespace().get_var_type(self)

    def getPointerReference(self):
        return EnkfConfigNode.cNamespace().get_ref(self)

    def get_min_std_file(self):
        return EnkfConfigNode.cNamespace().get_min_std_file(self)

    def get_enkf_outfile(self):
        return EnkfConfigNode.cNamespace().get_enkf_outfile(self)

    def getFieldModelConfig(self):
        return FieldConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    def getDataModelConfig(self):
        return GenDataConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    def getKeywordModelConfig(self):
        return GenKwConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    def get_enkf_infile(self):
        return EnkfConfigNode.cNamespace().get_enkf_infile(self)

    def alloc_node(self):
        return EnkfNode(self)

    def get_init_file_fmt(self):
        return EnkfConfigNode.cNamespace().get_init_file_fmt(self)

    def free(self):
        EnkfConfigNode.cNamespace().free(self)

    def getModelConfig(self):
        implementation_type = self.getImplementationType()

        if implementation_type == self.FIELD:
            return self.getFieldModelConfig()
        elif implementation_type == self.GEN_DATA:
            return self.getDataModelConfig()
        elif implementation_type == self.GEN_KW:
            return self.getKeywordModelConfig()
        elif implementation_type == self.SUMMARY:
            return SummaryConfig.createCReference(self.getPointerReference(), parent=self)
        else:
            print("[EnkfConfigNode::getModelConfig()] Unhandled implementation model type: %i" % implementation_type)
            # raise NotImplementedError("Unknown model type: %i" % type)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_config_node", EnkfConfigNode)
cwrapper.registerType("enkf_config_node_obj", EnkfConfigNode.createPythonObject)
cwrapper.registerType("enkf_config_node_ref", EnkfConfigNode.createCReference)

EnkfConfigNode.cNamespace().free = cwrapper.prototype("void enkf_config_node_free( enkf_config_node )")
EnkfConfigNode.cNamespace().get_ref = cwrapper.prototype("c_void_p enkf_config_node_get_ref(enkf_config_node)") #todo: fix return type
EnkfConfigNode.cNamespace().get_impl_type = cwrapper.prototype("int enkf_config_node_get_impl_type(enkf_config_node)") #todo: fix return type as enum
EnkfConfigNode.cNamespace().get_enkf_outfile = cwrapper.prototype("char* enkf_config_node_get_enkf_outfile(enkf_config_node)")
EnkfConfigNode.cNamespace().get_min_std_file = cwrapper.prototype("char* enkf_config_node_get_min_std_file(enkf_config_node)")
EnkfConfigNode.cNamespace().get_enkf_infile = cwrapper.prototype("char* enkf_config_node_get_enkf_infile(enkf_config_node)")
EnkfConfigNode.cNamespace().get_init_file_fmt = cwrapper.prototype("char* enkf_config_node_get_init_file_fmt(enkf_config_node)")
EnkfConfigNode.cNamespace().get_var_type = cwrapper.prototype("c_void_p enkf_config_node_get_var_type(enkf_config_node)") #todo: fix return type as enum
