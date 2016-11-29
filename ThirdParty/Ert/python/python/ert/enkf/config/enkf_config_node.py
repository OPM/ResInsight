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
from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.config import FieldConfig, GenDataConfig, GenKwConfig, SummaryConfig, CustomKWConfig
from ert.enkf.enums import EnkfTruncationType, ErtImplType, LoadFailTypeEnum, EnkfVarType
from ert.ecl import EclGrid



class EnkfConfigNode(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getImplementationType( self ):
        """ @rtype: ErtImplType """
        return EnkfConfigNode.cNamespace().get_impl_type(self)

    def getVariableType( self ):
         return EnkfConfigNode.cNamespace().get_var_type(self)

    def getPointerReference(self):
        return EnkfConfigNode.cNamespace().get_ref(self)

    # def get_min_std_file(self):
    #     return EnkfConfigNode.cNamespace().get_min_std_file(self)

    # def get_enkf_outfile(self):
    #     return EnkfConfigNode.cNamespace().get_enkf_outfile(self)

    def getFieldModelConfig(self):
        """ @rtype: FieldConfig """
        return FieldConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    def getDataModelConfig(self):
        """ @rtype: GenDataConfig """
        return GenDataConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    def getKeywordModelConfig(self):
        """ @rtype: GenKWConfig """
        return GenKwConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    def getCustomKeywordModelConfig(self):
        """ @rtype: CustomKWConfig """
        return CustomKWConfig.createCReference(EnkfConfigNode.cNamespace().get_ref(self), parent=self)

    # def get_enkf_infile(self):
    #     return EnkfConfigNode.cNamespace().get_enkf_infile(self)

    # def alloc_node(self):
    #     return EnkfNode(self)

    # def get_init_file_fmt(self):
    #     return EnkfConfigNode.cNamespace().get_init_file_fmt(self)

    def getObservationKeys(self):
        """ @rtype:  StringList """
        return EnkfConfigNode.cNamespace().get_obs_keys(self).setParent(self)

    def updateStateField(self, truncation, value_min, value_max):
        assert isinstance(truncation, EnkfTruncationType)
        EnkfConfigNode.cNamespace().update_state_field(self, truncation, value_min, value_max)

    @classmethod
    def createSummaryConfigNode(cls, key, load_fail_type):
        """
         @type key: str
         @type load_fail_type: LoadFailTypeEnum
        @rtype: EnkfConfigNode
        """

        assert isinstance(load_fail_type, LoadFailTypeEnum)
        return EnkfConfigNode.cNamespace().alloc_summary_node(key, load_fail_type)

    @classmethod
    def createFieldConfigNode(cls, key, grid, trans_table = None, forward_init = False):
        """
        @type grid: EclGrid
        @rtype: EnkfConfigNode
        """
        return EnkfConfigNode.cNamespace().alloc_field_node(key, grid, trans_table, forward_init)

    def free(self):
        EnkfConfigNode.cNamespace().free(self)

    def getModelConfig(self):
        implementation_type = self.getImplementationType()

        if implementation_type == ErtImplType.FIELD:
            return self.getFieldModelConfig()
        elif implementation_type == ErtImplType.GEN_DATA:
            return self.getDataModelConfig()
        elif implementation_type == ErtImplType.GEN_KW:
            return self.getKeywordModelConfig()
        elif implementation_type == ErtImplType.CUSTOM_KW:
            return self.getCustomKeywordModelConfig()
        elif implementation_type == ErtImplType.SUMMARY:
            return SummaryConfig.createCReference(self.getPointerReference(), parent=self)
        else:
            print("[EnkfConfigNode::getModelConfig()] Unhandled implementation model type: %i" % implementation_type)
            # raise NotImplementedError("Unknown model type: %i" % type)

    def getKey(self):
        return EnkfConfigNode.cNamespace().get_key( self )


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("enkf_config_node", EnkfConfigNode)

EnkfConfigNode.cNamespace().free = cwrapper.prototype("void enkf_config_node_free(enkf_config_node)")
EnkfConfigNode.cNamespace().get_ref = cwrapper.prototype("c_void_p enkf_config_node_get_ref(enkf_config_node)") #todo: fix return type
EnkfConfigNode.cNamespace().get_impl_type = cwrapper.prototype("ert_impl_type_enum enkf_config_node_get_impl_type(enkf_config_node)")

EnkfConfigNode.cNamespace().get_enkf_outfile = cwrapper.prototype("char* enkf_config_node_get_enkf_outfile(enkf_config_node)")
EnkfConfigNode.cNamespace().get_min_std_file = cwrapper.prototype("char* enkf_config_node_get_min_std_file(enkf_config_node)")
EnkfConfigNode.cNamespace().get_enkf_infile = cwrapper.prototype("char* enkf_config_node_get_enkf_infile(enkf_config_node)")
EnkfConfigNode.cNamespace().get_init_file_fmt = cwrapper.prototype("char* enkf_config_node_get_init_file_fmt(enkf_config_node)")
EnkfConfigNode.cNamespace().get_var_type = cwrapper.prototype("enkf_var_type_enum enkf_config_node_get_var_type(enkf_config_node)") #todo: fix return type as enum
EnkfConfigNode.cNamespace().get_key = cwrapper.prototype("char* enkf_config_node_get_key(enkf_config_node)") 
EnkfConfigNode.cNamespace().get_obs_keys = cwrapper.prototype("stringlist_ref enkf_config_node_get_obs_keys(enkf_config_node)")
EnkfConfigNode.cNamespace().alloc_summary_node = cwrapper.prototype("enkf_config_node_obj enkf_config_node_alloc_summary(char*, load_fail_type)")
EnkfConfigNode.cNamespace().alloc_field_node = cwrapper.prototype("enkf_config_node_obj enkf_config_node_alloc_field(char*, ecl_grid, c_void_p, bool)")
EnkfConfigNode.cNamespace().update_state_field = cwrapper.prototype("void enkf_config_node_update_state_field(enkf_config_node, enkf_truncation_type_enum, double, double)")
