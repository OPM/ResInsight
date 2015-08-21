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

    
    def addTimeStep(self , step):
        LocalObsdataNode.cNamespace().add_step(self, step )


    def free(self):
        LocalObsdataNode.cNamespace().free(self)


    def getStepList(self):
        if self.allTimeStepActive():
            raise ValueError("LocalObsdataNode configured with all time step active - can not ask for step list")
        return LocalObsdataNode.cNamespace().get_step_list( self )


    def getActiveList(self):
        return LocalObsdataNode.cNamespace().get_active_list( self )

    def allTimeStepActive(self):
        return LocalObsdataNode.cNamespace().all_timestep_active( self )

    

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("local_obsdata_node", LocalObsdataNode)

LocalObsdataNode.cNamespace().alloc            = cwrapper.prototype("c_void_p local_obsdata_node_alloc(char*)")
LocalObsdataNode.cNamespace().free             = cwrapper.prototype("void local_obsdata_node_free(local_obsdata_node)")
LocalObsdataNode.cNamespace().get_key          = cwrapper.prototype("char* local_obsdata_node_get_key(local_obsdata_node)")
LocalObsdataNode.cNamespace().add_range        = cwrapper.prototype("void local_obsdata_node_add_range(local_obsdata_node, int, int)")
LocalObsdataNode.cNamespace().add_step         = cwrapper.prototype("void local_obsdata_node_add_tstep(local_obsdata_node, int)")
LocalObsdataNode.cNamespace().get_step_list    = cwrapper.prototype("int_vector_ref local_obsdata_node_get_tstep_list(local_obsdata_node)")
LocalObsdataNode.cNamespace().get_active_list  = cwrapper.prototype("active_list_ref local_obsdata_node_get_active_list(local_obsdata_node)")
LocalObsdataNode.cNamespace().all_timestep_active  = cwrapper.prototype("bool local_obsdata_node_all_timestep_active(local_obsdata_node)")


