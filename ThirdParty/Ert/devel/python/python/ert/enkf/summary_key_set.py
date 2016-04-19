from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB
from ert.util import StringList


class SummaryKeySet(BaseCClass):

    def __init__(self, filename=None, read_only=False):
        if filename is None:
            c_ptr = SummaryKeySet.cNamespace().alloc()
        else:
            c_ptr = SummaryKeySet.cNamespace().alloc_from_file(filename, read_only)

        super(SummaryKeySet, self).__init__(c_ptr)

    def addSummaryKey(self, key):
        assert isinstance(key, str)
        return SummaryKeySet.cNamespace().add_key(self, key)

    def __len__(self):
        return SummaryKeySet.cNamespace().size(self)

    def __contains__(self, key):
        return SummaryKeySet.cNamespace().has_key(self, key)

    def keys(self):
        """ @rtype: StringList """
        return SummaryKeySet.cNamespace().keys(self)

    def isReadOnly(self):
        """ @rtype: bool """
        return SummaryKeySet.cNamespace().is_read_only(self)


    def writeToFile(self, filename):
        assert isinstance(filename, str)
        SummaryKeySet.cNamespace().fwrite(self, filename)

    def free(self):
        SummaryKeySet.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("summary_key_set", SummaryKeySet)

SummaryKeySet.cNamespace().alloc  = cwrapper.prototype("c_void_p summary_key_set_alloc()")
SummaryKeySet.cNamespace().alloc_from_file  = cwrapper.prototype("c_void_p summary_key_set_alloc_from_file(char*, bool)")
SummaryKeySet.cNamespace().free  = cwrapper.prototype("void summary_key_set_free(summary_key_set)")
SummaryKeySet.cNamespace().size  = cwrapper.prototype("int summary_key_set_get_size(summary_key_set)")
SummaryKeySet.cNamespace().add_key  = cwrapper.prototype("bool summary_key_set_add_summary_key(summary_key_set, char*)")
SummaryKeySet.cNamespace().has_key  = cwrapper.prototype("bool summary_key_set_has_summary_key(summary_key_set, char*)")
SummaryKeySet.cNamespace().keys  = cwrapper.prototype("stringlist_obj summary_key_set_alloc_keys(summary_key_set)")
SummaryKeySet.cNamespace().is_read_only  = cwrapper.prototype("bool summary_key_set_is_read_only(summary_key_set)")
SummaryKeySet.cNamespace().fwrite  = cwrapper.prototype("void summary_key_set_fwrite(summary_key_set, char*)")
