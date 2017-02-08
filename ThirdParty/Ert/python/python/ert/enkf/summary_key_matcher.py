from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import StringList


class SummaryKeyMatcher(BaseCClass):
    TYPE_NAME = "summary_key_matcher"

    _alloc       = EnkfPrototype("void* summary_key_matcher_alloc()", bind = False)
    _free        = EnkfPrototype("void  summary_key_matcher_free(summary_key_matcher)")
    _size        = EnkfPrototype("int   summary_key_matcher_get_size(summary_key_matcher)")
    _add_key     = EnkfPrototype("void  summary_key_matcher_add_summary_key(summary_key_matcher, char*)")
    _match_key   = EnkfPrototype("bool  summary_key_matcher_match_summary_key(summary_key_matcher, char*)")
    _keys        = EnkfPrototype("stringlist_obj summary_key_matcher_get_keys(summary_key_matcher)")
    _is_required = EnkfPrototype("bool  summary_key_matcher_summary_key_is_required(summary_key_matcher, char*)")

    def __init__(self):
        c_ptr = self._alloc()

        super(SummaryKeyMatcher, self).__init__(c_ptr)

    def addSummaryKey(self, key):
        assert isinstance(key, str)
        return self._add_key(key)

    def __len__(self):
        return self._size()

    def __contains__(self, key):
        return self._match_key(key)

    def isRequired(self, key):
        """ @rtype: bool """
        return self._is_required(key)

    def keys(self):
        """ @rtype: StringList """
        return self._keys()

    def free(self):
        self._free()
