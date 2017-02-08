from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.config.custom_kw_config import CustomKWConfig


class CustomKWConfigSet(BaseCClass):
    TYPE_NAME = "custom_kw_config_set"

    _alloc           = EnkfPrototype("void* custom_kw_config_set_alloc()", bind = False)
    _alloc_from_file = EnkfPrototype("void* custom_kw_config_set_alloc_from_file(char*)", bind = False)
    _free            = EnkfPrototype("void  custom_kw_config_set_free(custom_kw_config_set)")
    _reset           = EnkfPrototype("void  custom_kw_config_set_reset(custom_kw_config_set)")
    _add_config      = EnkfPrototype("void  custom_kw_config_set_add_config(custom_kw_config_set, custom_kw_config)")
    _update_config   = EnkfPrototype("void  custom_kw_config_set_update_config(custom_kw_config_set, custom_kw_config)")
    _fwrite          = EnkfPrototype("void  custom_kw_config_set_fwrite(custom_kw_config_set, char*)")
    _get_keys        = EnkfPrototype("stringlist_obj custom_kw_config_set_get_keys_alloc(custom_kw_config_set)")

    def __init__(self, filename=None):
        if filename is None:
            c_ptr = self._alloc()
        else:
            c_ptr = self._alloc_from_file(filename)
        super(CustomKWConfigSet, self).__init__(c_ptr)


    def addConfig(self, config):
        """ @type config: CustomKWConfig """
        assert isinstance(config, CustomKWConfig)
        self._add_config(config)

    def getStoredConfigKeys(self):
        """ @rtype: StringList """
        return self._get_keys()

    def updateConfig(self, config):
        """ @type config: CustomKWConfig """
        self._update_config(config)

    def fwrite(self, filename):
        """ @type filename: str """
        self._fwrite(filename)

    def reset(self):
        self._reset()

    def free(self):
        self._free()
