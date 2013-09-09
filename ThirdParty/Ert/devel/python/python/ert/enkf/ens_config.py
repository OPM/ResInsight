#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ens_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.util import StringList


class EnsConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def has_key(self, key):
        """ @rtype: bool """
        return EnsConfig.cNamespace().has_key(self, key)

    def get_node(self, key):
        """ @rtype: EnkfConfigNode """
        return EnsConfig.cNamespace().get_node(self, key).setParent(self)

    def alloc_keylist(self):
        """ @rtype: StringList """
        return EnsConfig.cNamespace().alloc_keylist(self).setParent(self)

    def add_summary(self, key):
        """ @rtype: EnkfConfigNode """
        return EnsConfig.cNamespace().add_summary(self, key, 2).setParent(self)

    def add_gen_kw(self, key):
        """ @rtype: EnkfConfigNode """
        return EnsConfig.cNamespace().add_gen_kw(self, key).setParent(self)

    def add_gen_data(self, key):
        """ @rtype: EnkfConfigNode """
        return EnsConfig.cNamespace().add_gen_data(self, key).setParent(self)

    def add_field(self, key, eclipse_grid):
        """ @rtype: EnkfConfigNode """
        return EnsConfig.cNamespace().add_field(self, key, eclipse_grid).setParent(self)

    def alloc_keylist_from_var_type(self, var_mask):
        """ @rtype: StringList """
        return EnsConfig.cNamespace().alloc_keylist_from_var_type(self, var_mask).setParent(self)

    def free(self):
        EnsConfig.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ens_config", EnsConfig)
cwrapper.registerType("ens_config_obj", EnsConfig.createPythonObject)
cwrapper.registerType("ens_config_ref", EnsConfig.createCReference)

EnsConfig.cNamespace().free = cwrapper.prototype("void ensemble_config_free( ens_config )")
EnsConfig.cNamespace().has_key = cwrapper.prototype("bool ensemble_config_has_key( ens_config , char* )")
EnsConfig.cNamespace().get_node = cwrapper.prototype("enkf_config_node_ref ensemble_config_get_node( ens_config , char*)")
EnsConfig.cNamespace().alloc_keylist = cwrapper.prototype("stringlist_ref ensemble_config_alloc_keylist( ens_config )")
EnsConfig.cNamespace().add_summary = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_summary( ens_config, char*, int)")
EnsConfig.cNamespace().add_gen_kw = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_gen_kw( ens_config, char*)")
EnsConfig.cNamespace().add_gen_data = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_gen_data( ens_config, char*)")
EnsConfig.cNamespace().add_field = cwrapper.prototype("enkf_config_node_ref ensemble_config_add_field( ens_config, char*, ecl_grid)")
EnsConfig.cNamespace().alloc_keylist_from_var_type = cwrapper.prototype("stringlist_ref ensemble_config_alloc_keylist_from_var_type(ens_config, int)")
