from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB

class LocalObsdataNode(BaseCClass):

    def __init__(self, obs_key , all_timestep_active = True):
        assert isinstance(obs_key, str)

        c_pointer = LocalObsdataNode.cNamespace().alloc(obs_key , all_timestep_active)
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

    def tstepActive(self , tstep):
        return LocalObsdataNode.cNamespace().tstep_active( self , tstep)


    def getActiveList(self):
        return LocalObsdataNode.cNamespace().get_active_list( self )

    def allTimeStepActive(self):
        return LocalObsdataNode.cNamespace().all_timestep_active( self )

    def setAllTimeStepActive(self, flag):
        return LocalObsdataNode.cNamespace().set_all_timestep_active( self, flag )

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("local_obsdata_node", LocalObsdataNode)

LocalObsdataNode.cNamespace().alloc            = cwrapper.prototype("c_void_p local_obsdata_node_alloc(char* , bool)")
LocalObsdataNode.cNamespace().free             = cwrapper.prototype("void local_obsdata_node_free(local_obsdata_node)")
LocalObsdataNode.cNamespace().get_key          = cwrapper.prototype("char* local_obsdata_node_get_key(local_obsdata_node)")
LocalObsdataNode.cNamespace().add_range        = cwrapper.prototype("void local_obsdata_node_add_range(local_obsdata_node, int, int)")
LocalObsdataNode.cNamespace().add_step         = cwrapper.prototype("void local_obsdata_node_add_tstep(local_obsdata_node, int)")
LocalObsdataNode.cNamespace().tstep_active     = cwrapper.prototype("bool local_obsdata_node_tstep_active(local_obsdata_node, int)")
LocalObsdataNode.cNamespace().get_active_list  = cwrapper.prototype("active_list_ref local_obsdata_node_get_active_list(local_obsdata_node)")
LocalObsdataNode.cNamespace().all_timestep_active  = cwrapper.prototype("bool local_obsdata_node_all_timestep_active(local_obsdata_node)")
LocalObsdataNode.cNamespace().set_all_timestep_active  = cwrapper.prototype("void local_obsdata_node_set_all_timestep_active(local_obsdata_node, bool)")


