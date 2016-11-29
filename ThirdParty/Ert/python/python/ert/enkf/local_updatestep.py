from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, LocalMinistep

class LocalUpdateStep(BaseCClass):

    def __init__(self, updatestep_key):
        raise NotImplementedError("Class can not be instantiated directly!")
    
    def __len__(self):
        """ @rtype: int """
        return LocalUpdateStep.cNamespace().size(self)
    
    def __getitem__(self, index):
        """ @rtype: LocalMinistep """
        assert isinstance(index, int)
        if index < len(self):
            return LocalUpdateStep.cNamespace().iget_ministep(self, index)
        else:
            raise IndexError("Invalid index")
        
    def attachMinistep(self, ministep):
        assert isinstance(ministep, LocalMinistep)
        LocalUpdateStep.cNamespace().attach_ministep(self,ministep)
                    
    def getName(self):
        """ @rtype: str """
        return LocalUpdateStep.cNamespace().name(self)
                       
    def free(self):
        LocalUpdateStep.cNamespace().free(self) 

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("local_updatestep", LocalUpdateStep)

LocalUpdateStep.cNamespace().alloc               = cwrapper.prototype("c_void_p local_updatestep_alloc(char*)")
LocalUpdateStep.cNamespace().size                = cwrapper.prototype("int local_updatestep_get_num_ministep(local_updatestep)")
LocalUpdateStep.cNamespace().iget_ministep       = cwrapper.prototype("local_ministep_ref local_updatestep_iget_ministep(local_updatestep, int)")
LocalUpdateStep.cNamespace().free                = cwrapper.prototype("void local_updatestep_free(local_updatestep)")
LocalUpdateStep.cNamespace().attach_ministep     = cwrapper.prototype("void local_updatestep_add_ministep(local_updatestep,local_ministep)")
LocalUpdateStep.cNamespace().name                = cwrapper.prototype("char* local_updatestep_get_name(local_updatestep)")


