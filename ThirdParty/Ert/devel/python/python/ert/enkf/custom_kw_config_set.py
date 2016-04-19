from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB
from ert.enkf.config.custom_kw_config import CustomKWConfig


class CustomKWConfigSet(BaseCClass):

    def __init__(self, filename=None):
        if filename is None:
            c_ptr = CustomKWConfigSet.cNamespace().alloc()
        else:
            c_ptr = CustomKWConfigSet.cNamespace().alloc_from_file(filename)

        super(CustomKWConfigSet, self).__init__(c_ptr)


    def addConfig(self, config):
        """ @type config: CustomKWConfig """
        assert isinstance(config, CustomKWConfig)
        CustomKWConfigSet.cNamespace().add_config(self, config)

    def getStoredConfigKeys(self):
        """ @rtype: StringList """
        return CustomKWConfigSet.cNamespace().get_keys(self)

    def updateConfig(self, config):
        """ @type config: CustomKWConfig """
        CustomKWConfigSet.cNamespace().update_config(self, config)

    def fwrite(self, filename):
        """ @type filename: str """
        CustomKWConfigSet.cNamespace().fwrite(self, filename)

    def reset(self):
        CustomKWConfigSet.cNamespace().reset(self)

    def free(self):
        CustomKWConfigSet.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("custom_kw_config_set", CustomKWConfigSet)

CustomKWConfigSet.cNamespace().alloc  = cwrapper.prototype("c_void_p custom_kw_config_set_alloc()")
CustomKWConfigSet.cNamespace().alloc_from_file  = cwrapper.prototype("c_void_p custom_kw_config_set_alloc_from_file(char*)")
CustomKWConfigSet.cNamespace().free  = cwrapper.prototype("void custom_kw_config_set_free(custom_kw_config_set)")
CustomKWConfigSet.cNamespace().reset  = cwrapper.prototype("void custom_kw_config_set_reset(custom_kw_config_set)")
CustomKWConfigSet.cNamespace().add_config  = cwrapper.prototype("void custom_kw_config_set_add_config(custom_kw_config_set, custom_kw_config)")
CustomKWConfigSet.cNamespace().update_config  = cwrapper.prototype("void custom_kw_config_set_update_config(custom_kw_config_set, custom_kw_config)")
CustomKWConfigSet.cNamespace().get_keys  = cwrapper.prototype("stringlist_obj custom_kw_config_set_get_keys_alloc(custom_kw_config_set)")
CustomKWConfigSet.cNamespace().fwrite  = cwrapper.prototype("void custom_kw_config_set_fwrite(custom_kw_config_set, char*)")
