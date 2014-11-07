from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, LocalObsdataNode


class LocalObsdata(BaseCClass):

    def __init__(self, name):
        assert isinstance(name, str)

        c_pointer = LocalObsdata.cNamespace().alloc(name)
        super(LocalObsdata, self).__init__(c_pointer)

    def __len__(self):
       """ @rtype: int """
       return LocalObsdata.cNamespace().size(self)


    def __getitem__(self, index):
       """ @rtype: LocalObsdataNode """
       assert isinstance(index, int)
       return LocalObsdata.cNamespace().get_node(self, index).setParent(self)

    def __iter__(self):
       cur = 0
       while cur < len(self):
           yield self[cur]
           cur += 1

    def addNode(self, node):
        """ @rtype: bool """
        assert isinstance(node, LocalObsdataNode)
        node.convertToCReference(self)
        already_exists_node_for_key = LocalObsdata.cNamespace().add_node(self, node)
        return already_exists_node_for_key

    def __contains__(self, item):
        """ @rtype: bool """
        if isinstance(item, str):
            return LocalObsdata.cNamespace().has_node(self, item)
        elif isinstance(item, LocalObsdataNode):
            return LocalObsdata.cNamespace().has_node(self, item.getKey())

        return False

    def getName(self):
        """ @rtype: str """
        return LocalObsdata.cNamespace().name(self)

    def free(self):
        LocalObsdata.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("local_obsdata", LocalObsdata)
cwrapper.registerType("local_obsdata_obj", LocalObsdata.createPythonObject)
cwrapper.registerType("local_obsdata_ref", LocalObsdata.createCReference)

LocalObsdata.cNamespace().alloc    = cwrapper.prototype("c_void_p local_obsdata_alloc(char*)")
LocalObsdata.cNamespace().free     = cwrapper.prototype("void local_obsdata_free(local_obsdata)")
LocalObsdata.cNamespace().size     = cwrapper.prototype("int local_obsdata_get_size(local_obsdata)")
LocalObsdata.cNamespace().has_node = cwrapper.prototype("bool local_obsdata_has_node(local_obsdata, char*)")
LocalObsdata.cNamespace().add_node = cwrapper.prototype("bool local_obsdata_add_node(local_obsdata, local_obsdata_node)")
LocalObsdata.cNamespace().get_node = cwrapper.prototype("local_obsdata_node_ref local_obsdata_iget(local_obsdata, int)")
LocalObsdata.cNamespace().name     = cwrapper.prototype("char* local_obsdata_get_name(local_obsdata)")



