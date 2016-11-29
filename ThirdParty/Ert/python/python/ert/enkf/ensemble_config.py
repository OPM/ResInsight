#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ensemble_config.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.util import StringList
from ert.enkf import ENKF_LIB, SummaryKeyMatcher
from ert.enkf.config import EnkfConfigNode, CustomKWConfig
from ert.enkf.enums import EnkfVarType, ErtImplType


class EnsembleConfig(BaseCClass):

    def __init__(self):
        c_ptr = EnsembleConfig.cNamespace().alloc()
        super(EnsembleConfig , self).__init__(c_ptr)
        

    def __len__(self):
        return EnsembleConfig.cNamespace().size( self )

    
    def __getitem__(self , key):
        """ @rtype: EnkfConfigNode """
        if key in self:
            return EnsembleConfig.cNamespace().get_node(self, key).setParent(self)
        else:
            raise KeyError("The key:%s is not in the ensemble configuration" % key)

    
    def getNode(self, key):
        return self[key]


    def alloc_keylist(self):
        """ @rtype: StringList """
        return EnsembleConfig.cNamespace().alloc_keylist(self)

    def add_summary(self, key):
        """ @rtype: EnkfConfigNode """
        return EnsembleConfig.cNamespace().add_summary(self, key, 2).setParent(self)

    def add_gen_kw(self, key):
        """ @rtype: EnkfConfigNode """
        return EnsembleConfig.cNamespace().add_gen_kw(self, key).setParent(self)


    def addNode(self , config_node):
        assert isinstance(config_node , EnkfConfigNode)
        EnsembleConfig.cNamespace().add_node( self , config_node )
        config_node.setParent( self )


    def add_field(self, key, eclipse_grid):
        """ @rtype: EnkfConfigNode """
        return EnsembleConfig.cNamespace().add_field(self, key, eclipse_grid).setParent(self)

    def getKeylistFromVarType(self, var_mask):
        """ @rtype: StringList """
        assert isinstance(var_mask, EnkfVarType)
        return EnsembleConfig.cNamespace().alloc_keylist_from_var_type(self, var_mask)

    def getKeylistFromImplType(self, ert_impl_type):
        """ @rtype: StringList """
        assert isinstance(ert_impl_type, ErtImplType)
        return EnsembleConfig.cNamespace().alloc_keylist_from_impl_type(self, ert_impl_type)

    def __contains__(self, key):
        return EnsembleConfig.cNamespace().has_key(self, key)

    def getSummaryKeyMatcher(self):
        """ @rtype: SummaryKeyMatcher """
        return EnsembleConfig.cNamespace().summary_key_matcher(self)

    def free(self):
        EnsembleConfig.cNamespace().free(self)

    def addDefinedCustomKW(self, group_name, definition):
        """ @rtype: EnkfConfigNode """
        if not group_name in self:
            type_hash = CustomKWConfig.convertDefinition(definition)
            EnsembleConfig.cNamespace().add_defined_custom_kw(self, group_name, type_hash)

        return self[group_name]



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("ens_config", EnsembleConfig)

EnsembleConfig.cNamespace().alloc = cwrapper.prototype("c_void_p ensemble_config_alloc(  )")
EnsembleConfig.cNamespace().free = cwrapper.prototype("void ensemble_config_free( ens_config )")
EnsembleConfig.cNamespace().has_key = cwrapper.prototype("bool ensemble_config_has_key( ens_config , char* )")
EnsembleConfig.cNamespace().size = cwrapper.prototype("int ensemble_config_get_size( ens_config)")
EnsembleConfig.cNamespace().get_node = cwrapper.prototype("enkf_config_node_ref ensemble_config_get_node( ens_config , char*)")
EnsembleConfig.cNamespace().alloc_keylist = cwrapper.prototype("stringlist_obj ensemble_config_alloc_keylist( ens_config )")
EnsembleConfig.cNamespace().add_summary = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_summary( ens_config, char*, int)")
EnsembleConfig.cNamespace().add_gen_kw = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_gen_kw( ens_config, char*)")
EnsembleConfig.cNamespace().add_field = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_field( ens_config, char*, ecl_grid)")
EnsembleConfig.cNamespace().alloc_keylist_from_var_type = cwrapper.prototype("stringlist_obj ensemble_config_alloc_keylist_from_var_type(ens_config, enkf_var_type_enum)")
EnsembleConfig.cNamespace().alloc_keylist_from_impl_type = cwrapper.prototype("stringlist_obj ensemble_config_alloc_keylist_from_impl_type(ens_config, ert_impl_type_enum)")
EnsembleConfig.cNamespace().add_node = cwrapper.prototype("void ensemble_config_add_node( ens_config , enkf_config_node )")
EnsembleConfig.cNamespace().summary_key_matcher = cwrapper.prototype("summary_key_matcher_ref ensemble_config_get_summary_key_matcher(ens_config)")

EnsembleConfig.cNamespace().add_defined_custom_kw = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_defined_custom_kw(ens_config, char*, integer_hash)")
