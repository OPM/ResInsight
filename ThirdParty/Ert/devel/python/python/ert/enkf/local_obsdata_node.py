from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB

class LocalObsdataNode(BaseCClass):

    def __init__(self, obs_key):
        assert isinstance(obs_key, str)

        c_pointer = LocalObsdataNode.cNamespace().alloc(obs_key)
        super(LocalObsdataNode, self).__init__(c_pointer)

    def getKey(self):
        return LocalObsdataNode.cNamespace().get_key(self)

    def addRange(self, step_1, step_2):
        assert isinstance(step_1, int)
        assert isinstance(step_2, int)
        LocalObsdataNode.cNamespace().add_range(self, step_1, step_2)

    
    def addActiveTstep(self , obs_vector):
        LocalObsdataNode.cNamespace().add_active_tstep(self , obs_vector)


    def addTimeStep(self , step):
        LocalObsdataNode.cNamespace().add_step(self, step )


    def free(self):
        LocalObsdataNode.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("local_obsdata_node", LocalObsdataNode)
cwrapper.registerType("local_obsdata_node_obj", LocalObsdataNode.createPythonObject)
cwrapper.registerType("local_obsdata_node_ref", LocalObsdataNode.createCReference)

LocalObsdataNode.cNamespace().alloc            = cwrapper.prototype("c_void_p local_obsdata_node_alloc(char*)")
LocalObsdataNode.cNamespace().free             = cwrapper.prototype("void local_obsdata_node_free(local_obsdata_node)")
LocalObsdataNode.cNamespace().get_key          = cwrapper.prototype("char* local_obsdata_node_get_key(local_obsdata_node)")
LocalObsdataNode.cNamespace().add_range        = cwrapper.prototype("void local_obsdata_node_add_range(local_obsdata_node, int, int)")
LocalObsdataNode.cNamespace().add_step         = cwrapper.prototype("void local_obsdata_node_add_tstep(local_obsdata_node, int)")
LocalObsdataNode.cNamespace().add_active_tstep = cwrapper.prototype("void local_obsdata_node_add_active_tstep(local_obsdata_node , obs_vector)")


