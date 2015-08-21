from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB
from ert.util import StringList


class SummaryKeyMatcher(BaseCClass):

    def __init__(self):
        c_ptr = SummaryKeyMatcher.cNamespace().alloc()

        super(SummaryKeyMatcher, self).__init__(c_ptr)

    def addSummaryKey(self, key):
        assert isinstance(key, str)
        return SummaryKeyMatcher.cNamespace().add_key(self, key)

    def __len__(self):
        return SummaryKeyMatcher.cNamespace().size(self)

    def __contains__(self, key):
        return SummaryKeyMatcher.cNamespace().match_key(self, key)

    def isRequired(self, key):
        """ @rtype: bool """
        return SummaryKeyMatcher.cNamespace().is_required(self, key)

    def keys(self):
        """ @rtype: StringList """
        return SummaryKeyMatcher.cNamespace().keys(self)

    def free(self):
        SummaryKeyMatcher.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("summary_key_matcher", SummaryKeyMatcher)

SummaryKeyMatcher.cNamespace().alloc  = cwrapper.prototype("c_void_p summary_key_matcher_alloc()")
SummaryKeyMatcher.cNamespace().free  = cwrapper.prototype("void summary_key_matcher_free(summary_key_matcher)")
SummaryKeyMatcher.cNamespace().size  = cwrapper.prototype("int summary_key_matcher_get_size(summary_key_matcher)")
SummaryKeyMatcher.cNamespace().add_key  = cwrapper.prototype("void summary_key_matcher_add_summary_key(summary_key_matcher, char*)")
SummaryKeyMatcher.cNamespace().match_key  = cwrapper.prototype("bool summary_key_matcher_match_summary_key(summary_key_matcher, char*)")
SummaryKeyMatcher.cNamespace().keys  = cwrapper.prototype("stringlist_obj summary_key_matcher_get_keys(summary_key_matcher)")
SummaryKeyMatcher.cNamespace().is_required  = cwrapper.prototype("bool summary_key_matcher_summary_key_is_required(summary_key_matcher, char*)")
