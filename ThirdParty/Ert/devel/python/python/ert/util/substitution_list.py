from ert.cwrap import BaseCClass, CWrapper
from ert.util import UTIL_LIB


class SubstitutionList(BaseCClass):
    def __init__(self):
        c_ptr = SubstitutionList.cNamespace().alloc(0)
        super(SubstitutionList, self).__init__(c_ptr)

    def __len__(self):
        return SubstitutionList.cNamespace().size(self)

    def addItem(self, key, value, doc_string=""):
        SubstitutionList.cNamespace().append_copy(self, key, value, doc_string)


    def __getitem__(self, index):
        if not isinstance(index, int):
            raise IndexError("Index must be a number!")

        if index < 0 or index >= len(self):
            raise IndexError("Index must be in the range: [%i, %i]" % (0, len(self) - 1))

        key =  SubstitutionList.cNamespace().get_key(self, index)
        value =  SubstitutionList.cNamespace().get_value(self, index)
        doc_string = SubstitutionList.cNamespace().get_doc_string(self, index)

        return key, value, doc_string

    def free(self):
        SubstitutionList.cNamespace().free(self)

cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerType("subst_list", SubstitutionList)
CWrapper.registerType("subst_list_obj", SubstitutionList.createPythonObject)
CWrapper.registerType("subst_list_ref", SubstitutionList.createCReference)

SubstitutionList.cNamespace().alloc = cwrapper.prototype("c_void_p subst_list_alloc(c_void_p)")
SubstitutionList.cNamespace().free = cwrapper.prototype("void subst_list_free(subst_list)")

SubstitutionList.cNamespace().size = cwrapper.prototype("int subst_list_get_size(subst_list)")
SubstitutionList.cNamespace().get_key = cwrapper.prototype("char* subst_list_iget_key(subst_list, int)")
SubstitutionList.cNamespace().get_value = cwrapper.prototype("char* subst_list_iget_value(subst_list, int)")
SubstitutionList.cNamespace().get_doc_string = cwrapper.prototype("char* subst_list_iget_doc_string(subst_list, int)")

SubstitutionList.cNamespace().append_copy = cwrapper.prototype("void subst_list_append_copy(subst_list, char*, char*, char*)")