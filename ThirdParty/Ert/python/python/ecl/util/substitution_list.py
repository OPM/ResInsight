from cwrap import BaseCClass
from ecl.util import UtilPrototype


class SubstitutionList(BaseCClass):
    TYPE_NAME = "subst_list"

    _alloc = UtilPrototype("void* subst_list_alloc(void*)" , bind = False)
    _free = UtilPrototype("void subst_list_free(subst_list)")
    _size = UtilPrototype("int subst_list_get_size(subst_list)")
    _get_key = UtilPrototype("char* subst_list_iget_key(subst_list, int)")
    _get_value = UtilPrototype("char* subst_list_iget_value(subst_list, int)")
    _get_doc_string = UtilPrototype("char* subst_list_iget_doc_string(subst_list, int)")
    _append_copy = UtilPrototype("void subst_list_append_copy(subst_list, char*, char*, char*)")

    def __init__(self):
        c_ptr = self._alloc(0)
        super(SubstitutionList, self).__init__(c_ptr)

    def __len__(self):
        return self._size()

    def addItem(self, key, value, doc_string=""):
        self._append_copy(key, value, doc_string)

    def __getitem__(self, index_or_key):
        if not isinstance(index_or_key, int):
            raise IndexError("Index must be a number!")

        if index_or_key < 0 or index_or_key >= len(self):
            raise IndexError("Index must be in the range: [%i, %i]" % (0, len(self) - 1))

        key = self._get_key(index_or_key)
        value = self._get_value(index_or_key)
        doc_string = self._get_doc_string(index_or_key)

        return key, value, doc_string

    def __iter__(self):
        index = 0
        while index < len(self):
            yield self[index]
            index += 1

    def __contains__(self, key):
        for kw, value, doc in self:
            if key == kw:
                return True
        return False

    def indexForKey(self, key):
        if not key in self:
            raise KeyError("Key '%s' not in substitution list!" % key)

        for index, key_val_doc in enumerate(self):
            if key == key_val_doc[0]:
                return index

        return None  # Should never happen!

    def free(self):
        self._free()
