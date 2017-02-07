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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.config import FieldConfig, GenDataConfig, GenKwConfig, SummaryConfig, CustomKWConfig
from ert.enkf.enums import EnkfTruncationType, ErtImplType, LoadFailTypeEnum, EnkfVarType
from ert.ecl import EclGrid
from ert.util import PathFormat

class EnkfConfigNode(BaseCClass):
    TYPE_NAME = "enkf_config_node"

    _alloc_summary_node = EnkfPrototype("enkf_config_node_obj enkf_config_node_alloc_summary(char*, load_fail_type)", bind = False)
    _alloc_field_node   = EnkfPrototype("enkf_config_node_obj enkf_config_node_alloc_field(char*, ecl_grid, void*, bool)", bind = False)
    _get_ref            = EnkfPrototype("void* enkf_config_node_get_ref(enkf_config_node)") #todo: fix return type
    _get_impl_type      = EnkfPrototype("ert_impl_type_enum enkf_config_node_get_impl_type(enkf_config_node)")
    _get_enkf_outfile   = EnkfPrototype("char* enkf_config_node_get_enkf_outfile(enkf_config_node)")
    _get_min_std_file   = EnkfPrototype("char* enkf_config_node_get_min_std_file(enkf_config_node)")
    _get_enkf_infile    = EnkfPrototype("char* enkf_config_node_get_enkf_infile(enkf_config_node)")
    _get_init_file      = EnkfPrototype("char* enkf_config_node_get_FIELD_fill_file(enkf_config_node, path_fmt)")
    _get_init_file_fmt  = EnkfPrototype("char* enkf_config_node_get_init_file_fmt(enkf_config_node)")
    _get_var_type       = EnkfPrototype("enkf_var_type_enum enkf_config_node_get_var_type(enkf_config_node)") #todo: fix return type as enum
    _get_key            = EnkfPrototype("char* enkf_config_node_get_key(enkf_config_node)")
    _get_obs_keys       = EnkfPrototype("stringlist_ref enkf_config_node_get_obs_keys(enkf_config_node)")
    _update_state_field = EnkfPrototype("void enkf_config_node_update_state_field(enkf_config_node, enkf_truncation_type_enum, double, double)")
    _free               = EnkfPrototype("void enkf_config_node_free(enkf_config_node)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getImplementationType( self ):
        """ @rtype: ErtImplType """
        return self._get_impl_type()

    def getVariableType( self ):
         return self._get_var_type()

    def getPointerReference(self):
        return self._get_ref()

    def getInitFile(self, model_config):
        return self._enkf_config_node_get_init_file(model_config.getRunpathFormat())

    # def get_min_std_file(self):
    #     return self._get_min_std_file()

    # def get_enkf_outfile(self):
    #     return self._get_enkf_outfile()

    def getFieldModelConfig(self):
        """ @rtype: FieldConfig """
        return FieldConfig.createCReference(self._get_ref(), parent=self)

    def getDataModelConfig(self):
        """ @rtype: GenDataConfig """
        return GenDataConfig.createCReference(self._get_ref(), parent=self)

    def getKeywordModelConfig(self):
        """ @rtype: GenKWConfig """
        return GenKwConfig.createCReference(self._get_ref(), parent=self)

    def getCustomKeywordModelConfig(self):
        """ @rtype: CustomKWConfig """
        return CustomKWConfig.createCReference(self._get_ref(), parent=self)

    # def get_enkf_infile(self):
    #     return self._get_enkf_infile()

    # def alloc_node(self):
    #     return EnkfNode(self)

    # def get_init_file_fmt(self):
    #     return self._get_init_file_fmt()

    def getObservationKeys(self):
        """ @rtype:  StringList """
        return self._get_obs_keys().setParent(self)

    def updateStateField(self, truncation, value_min, value_max):
        assert isinstance(truncation, EnkfTruncationType)
        self._update_state_field(truncation, value_min, value_max)

    @classmethod
    def createSummaryConfigNode(cls, key, load_fail_type):
        """
         @type key: str
         @type load_fail_type: LoadFailTypeEnum
        @rtype: EnkfConfigNode
        """

        assert isinstance(load_fail_type, LoadFailTypeEnum)
        return cls._alloc_summary_node(key, load_fail_type)

    @classmethod
    def createFieldConfigNode(cls, key, grid, trans_table = None, forward_init = False):
        """
        @type grid: EclGrid
        @rtype: EnkfConfigNode
        """
        return cls._alloc_field_node(key, grid, trans_table, forward_init)

    def free(self):
        self._free()

    def __repr__(self):
        key = self.getKey()
        vt  = self.getVariableType()
        imp = self.getImplementationType()
        content = 'key = %s, var_type = %s, implementation = %s' % (key, vt, imp)
        return self._create_repr(content)

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
        return self._get_key()
